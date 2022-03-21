//------------------------------------------------------------------------------
// GB_dense_subassign_05d_template: C<M> = x where C is as-if-full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get C and M
    //--------------------------------------------------------------------------

    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT (!C->iso) ;

    const int64_t *restrict Mp = M->p ;
    const int8_t  *restrict Mb = M->b ;
    const int64_t *restrict Mh = M->h ;
    const int64_t *restrict Mi = M->i ;
    const GB_void *restrict Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;
    const size_t mvlen = M->vlen ;

    GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    const int64_t cvlen = C->vlen ;

    const int64_t *restrict kfirst_Mslice = M_ek_slicing ;
    const int64_t *restrict klast_Mslice  = M_ek_slicing + M_ntasks ;
    const int64_t *restrict pstart_Mslice = M_ek_slicing + M_ntasks * 2 ;

    //--------------------------------------------------------------------------
    // C<M> = x
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(M_nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < M_ntasks ; taskid++)
    {

        // if kfirst > klast then taskid does no work at all
        int64_t kfirst = kfirst_Mslice [taskid] ;
        int64_t klast  = klast_Mslice  [taskid] ;

        //----------------------------------------------------------------------
        // C<M(:,kfirst:klast)> = x
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of M(:,k) to be operated on by this task
            //------------------------------------------------------------------

            int64_t j = GBH (Mh, k) ;
            int64_t pM_start, pM_end ;
            GB_get_pA (&pM_start, &pM_end, taskid, k,
                kfirst, klast, pstart_Mslice, Mp, mvlen) ;

            // pC points to the start of C(:,j) if C is dense
            int64_t pC = j * cvlen ;

            //------------------------------------------------------------------
            // C<M(:,j)> = x
            //------------------------------------------------------------------

            if (Mx == NULL && Mb == NULL)
            {
                GB_PRAGMA_SIMD_VECTORIZE
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                { 
                    int64_t p = pC + GBI (Mi, pM, mvlen) ;
                    GB_COPY_SCALAR_TO_C (p, cwork) ;        // Cx [p] = scalar
                }
            }
            else
            {
                GB_PRAGMA_SIMD_VECTORIZE
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                {
                    if (GBB (Mb, pM) && GB_mcast (Mx, pM, msize))
                    { 
                        int64_t p = pC + GBI (Mi, pM, mvlen) ;
                        GB_COPY_SCALAR_TO_C (p, cwork) ;    // Cx [p] = scalar
                    }
                }
            }
        }
    }
}

