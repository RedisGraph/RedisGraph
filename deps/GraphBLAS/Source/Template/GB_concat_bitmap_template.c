//------------------------------------------------------------------------------
// GB_concat_bitmap_template: concatenate a tile into a bitmap matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get C and the tile A
    //--------------------------------------------------------------------------

    #ifndef GB_ISO_CONCAT
    const bool A_iso = A->iso ;
    const GB_CTYPE *restrict Ax = (GB_CTYPE *) A->x ;
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif
    int8_t *restrict Cb = C->b ;

    //--------------------------------------------------------------------------
    // copy the tile A into C
    //--------------------------------------------------------------------------

    switch (GB_sparsity (A))
    {

        case GxB_FULL : // A is full
        {
            int A_nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
            int64_t pA ;
            #pragma omp parallel for num_threads(A_nthreads) schedule(static)
            for (pA = 0 ; pA < anz ; pA++)
            { 
                int64_t i = pA % avlen ;
                int64_t j = pA / avlen ;
                int64_t iC = cistart + i ;
                int64_t jC = cvstart + j ;
                int64_t pC = iC + jC * cvlen ;
                // Cx [pC] = Ax [pA] ;
                GB_COPY (pC, pA, A_iso) ;
                Cb [pC] = 1 ;
            }
        }
        break ;

        case GxB_BITMAP : // A is bitmap
        {
            int A_nthreads = GB_nthreads (anz, chunk, nthreads_max) ;
            const int8_t *restrict Ab = A->b ;
            int64_t pA ;
            #pragma omp parallel for num_threads(A_nthreads) schedule(static)
            for (pA = 0 ; pA < anz ; pA++)
            {
                if (Ab [pA])
                { 
                    int64_t i = pA % avlen ;
                    int64_t j = pA / avlen ;
                    int64_t iC = cistart + i ;
                    int64_t jC = cvstart + j ;
                    int64_t pC = iC + jC * cvlen ;
                    // Cx [pC] = Ax [pA] ;
                    GB_COPY (pC, pA, A_iso) ;
                    Cb [pC] = 1 ;
                }
            }
        }
        break ;

        default : // A is sparse or hypersparse
        {
            int A_nthreads, A_ntasks ;
            GB_SLICE_MATRIX (A, 1, chunk) ;
            const int64_t *restrict Ap = A->p ;
            const int64_t *restrict Ah = A->h ;
            const int64_t *restrict Ai = A->i ;
            int tid ;
            #pragma omp parallel for num_threads(A_nthreads) schedule(static)
            for (tid = 0 ; tid < A_ntasks ; tid++)
            {
                int64_t kfirst = kfirst_Aslice [tid] ;
                int64_t klast  = klast_Aslice  [tid] ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    int64_t j = GBH (Ah, k) ;
                    int64_t jC = cvstart + j ;
                    int64_t pC_start = cistart + jC * cvlen ;
                    int64_t pA_start, pA_end ;
                    GB_get_pA (&pA_start, &pA_end, tid, k,
                        kfirst, klast, pstart_Aslice, Ap, avlen) ;
                    GB_PRAGMA_SIMD
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    { 
                        int64_t i = Ai [pA] ;
                        int64_t pC = pC_start + i ;
                        // Cx [pC] = Ax [pA] ;
                        GB_COPY (pC, pA, A_iso) ;
                        Cb [pC] = 1 ;
                    }
                }
            }
        }
        break ;
    }

    done = true ;
}

#undef GB_CTYPE
#undef GB_ISO_CONCAT
