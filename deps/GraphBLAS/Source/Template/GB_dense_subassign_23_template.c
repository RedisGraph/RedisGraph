//------------------------------------------------------------------------------
// GB_dense_subassign_23_template: C += B where C is dense; B is sparse or dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_unused.h"

{

    //--------------------------------------------------------------------------
    // get C and B
    //--------------------------------------------------------------------------

    ASSERT (!C->iso) ;
    const GB_BTYPE *restrict Bx = (GB_BTYPE *) B->x ;
    const bool B_iso = B->iso ;
    GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    ASSERT (GB_is_dense (C)) ;
    const int64_t cnz = GB_nnz_held (C) ;

    if (GB_IS_BITMAP (B))
    {

        //----------------------------------------------------------------------
        // C += B when C is dense and B is bitmap
        //----------------------------------------------------------------------

        const int8_t *restrict Bb = B->b ;
        int64_t p ;
        #pragma omp parallel for num_threads(B_nthreads) schedule(static)
        for (p = 0 ; p < cnz ; p++)
        { 
            if (!Bb [p]) continue ;
            GB_GETB (bij, Bx, p, B_iso) ;                 // bij = B(i,j)
            GB_BINOP (GB_CX (p), GB_CX (p), bij, 0, 0) ;  // C(i,j) += bij
        }

    }
    else if (B_ek_slicing == NULL)
    {

        //----------------------------------------------------------------------
        // C += B when both C and B are dense
        //----------------------------------------------------------------------

        ASSERT (GB_is_dense (B)) ;
        int64_t p ;
        #pragma omp parallel for num_threads(B_nthreads) schedule(static)
        for (p = 0 ; p < cnz ; p++)
        { 
            GB_GETB (bij, Bx, p, B_iso) ;                 // bij = B(i,j)
            GB_BINOP (GB_CX (p), GB_CX (p), bij, 0, 0) ;  // C(i,j) += bij
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C += B when C is dense and B is sparse
        //----------------------------------------------------------------------

        ASSERT (GB_JUMBLED_OK (B)) ;

        const int64_t *restrict Bp = B->p ;
        const int64_t *restrict Bh = B->h ;
        const int64_t *restrict Bi = B->i ;
        const int64_t bvlen = B->vlen ;
        const int64_t cvlen = C->vlen ;
        bool B_jumbled = B->jumbled ;

        const int64_t *restrict kfirst_Bslice = B_ek_slicing ;
        const int64_t *restrict klast_Bslice  = kfirst_Bslice + B_ntasks ;
        const int64_t *restrict pstart_Bslice = klast_Bslice + B_ntasks ;

        int taskid ;
        #pragma omp parallel for num_threads(B_nthreads) schedule(dynamic,1)
        for (taskid = 0 ; taskid < B_ntasks ; taskid++)
        {

            // if kfirst > klast then taskid does no work at all
            int64_t kfirst = kfirst_Bslice [taskid] ;
            int64_t klast  = klast_Bslice  [taskid] ;

            //------------------------------------------------------------------
            // C(:,kfirst:klast) += B(:,kfirst:klast)
            //------------------------------------------------------------------

            for (int64_t k = kfirst ; k <= klast ; k++)
            {

                //--------------------------------------------------------------
                // find the part of B(:,k) and C(:,k) for this task
                //--------------------------------------------------------------

                int64_t j = GBH (Bh, k) ;
                int64_t my_pB_start, my_pB_end ;
                GB_get_pA (&my_pB_start, &my_pB_end, taskid, k,
                    kfirst, klast, pstart_Bslice, Bp, bvlen) ;

                int64_t pB_start = GBP (Bp, k, bvlen) ;
                int64_t pB_end   = GBP (Bp, k+1, bvlen) ;
                bool bjdense = ((pB_end - pB_start) == cvlen) ;

                // pC points to the start of C(:,j) if C is dense
                int64_t pC = j * cvlen ;

                //--------------------------------------------------------------
                // C(:,j) += B(:,j)
                //--------------------------------------------------------------

                if (bjdense && !B_jumbled)
                {

                    //----------------------------------------------------------
                    // both C(:,j) and B(:,j) are dense
                    //----------------------------------------------------------

                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t pB = my_pB_start ; pB < my_pB_end ; pB++)
                    { 
                        int64_t i = pB - pB_start ;
                        int64_t p = pC + i ;
                        GB_GETB (bij, Bx, pB, B_iso) ;          // bij = B(i,j)
                        // C(i,j) += bij
                        GB_BINOP (GB_CX (p), GB_CX (p), bij, 0, 0) ;
                    }

                }
                else
                {

                    //----------------------------------------------------------
                    // C(:,j) is dense; B(:,j) is sparse 
                    //----------------------------------------------------------

                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t pB = my_pB_start ; pB < my_pB_end ; pB++)
                    { 
                        int64_t i = Bi [pB] ;
                        int64_t p = pC + i ;
                        GB_GETB (bij, Bx, pB, B_iso) ;          // bij = B(i,j)
                        // C(i,j) += bij
                        GB_BINOP (GB_CX (p), GB_CX (p), bij, 0, 0) ;
                    }
                }
            }
        }
    }
}

