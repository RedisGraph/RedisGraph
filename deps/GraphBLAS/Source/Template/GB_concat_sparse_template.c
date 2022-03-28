//------------------------------------------------------------------------------
// GB_concat_sparse_template: concatenate a tile into a sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The tile A is hypersparse, sparse, or full, not bitmap.  If C is iso, then
// so is A, and the values are not copied here.

{

    //--------------------------------------------------------------------------
    // get C and the tile A
    //--------------------------------------------------------------------------

    #ifndef GB_ISO_CONCAT
    const GB_CTYPE *restrict Ax = (GB_CTYPE *) A->x ;
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif

    //--------------------------------------------------------------------------
    // copy the tile A into C
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(A_nthreads) schedule(static)
    for (tid = 0 ; tid < A_ntasks ; tid++)
    {
        int64_t kfirst = kfirst_Aslice [tid] ;
        int64_t klast  = klast_Aslice  [tid] ;
        for (int64_t k = kfirst ; k <= klast ; k++)
        {
            int64_t j = GBH (Ah, k) ;
            const int64_t pC_start = W [j] ;

            //------------------------------------------------------------------
            // find the part of the kth vector A(:,j) for this task
            //------------------------------------------------------------------

            int64_t pA_start, pA_end ;
            // as done by GB_get_pA, but also get p0 = Ap [k]
            const int64_t p0 = GBP (Ap, k, avlen) ;
            const int64_t p1 = GBP (Ap, k+1, avlen) ;
            if (k == kfirst)
            { 
                // First vector for task tid; may only be partially owned.
                pA_start = pstart_Aslice [tid] ;
                pA_end   = GB_IMIN (p1, pstart_Aslice [tid+1]) ;
            }
            else if (k == klast)
            { 
                // Last vector for task tid; may only be partially owned.
                pA_start = p0 ;
                pA_end   = pstart_Aslice [tid+1] ;
            }
            else
            { 
                // task tid entirely owns this vector A(:,k).
                pA_start = p0 ;
                pA_end   = p1 ;
            }

            //------------------------------------------------------------------
            // append A(:,j) onto C(:,j)
            //------------------------------------------------------------------

            GB_PRAGMA_SIMD
            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
            { 
                int64_t i = GBI (Ai, pA, avlen) ;       // i = Ai [pA]
                int64_t pC = pC_start + pA - p0 ;
                Ci [pC] = cistart + i ;
                // Cx [pC] = Ax [pA] ;
                GB_COPY (pC, pA, A_iso) ;
            }
        }
    }

    done = true ;
}

#undef GB_CTYPE
#undef GB_ISO_CONCAT

