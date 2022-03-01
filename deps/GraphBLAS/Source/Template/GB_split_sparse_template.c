//------------------------------------------------------------------------------
// GB_split_sparse_template: split a single tile from a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get A and C, and the slicing of C
    //--------------------------------------------------------------------------

    #ifndef GB_ISO_SPLIT
    const GB_CTYPE *restrict Ax = (GB_CTYPE *) A->x ;
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif

    //--------------------------------------------------------------------------
    // copy the tile from A to C
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(C_nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < C_ntasks ; tid++)
    {
        int64_t kfirst = kfirst_Cslice [tid] ;
        int64_t klast  = klast_Cslice  [tid] ;
        for (int64_t k = kfirst ; k <= klast ; k++)
        {
            // int64_t jA = GBH (Ah, k+akstart) ; not needed 
            int64_t pC_start, pC_end ;
            GB_get_pA (&pC_start, &pC_end, tid, k,
                kfirst, klast, pstart_Cslice, Cp, cvlen) ;
            int64_t p0 = Cp [k] ;
            int64_t pA_offset = Wp [k + akstart] ;
            // copy the vector from A to C
            for (int64_t pC = pC_start ; pC < pC_end ; pC++)
            { 
                // get the index of A(iA,jA)
                int64_t pA = pA_offset + pC - p0 ;
                int64_t iA = Ai [pA] ;
                // shift the index and copy into C(i,j)
                Ci [pC] = iA - aistart ;
                GB_COPY (pC, pA) ;
            }
        }
    }

    done = true ;
}

#undef GB_CTYPE
#undef GB_ISO_SPLIT

