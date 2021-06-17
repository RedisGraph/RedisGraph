//------------------------------------------------------------------------------
// GB_dense_subassign_05d_template: C<M> = x where C is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get C and M
    //--------------------------------------------------------------------------

    ASSERT (GB_JUMBLED_OK (M)) ;

    const int64_t *GB_RESTRICT Mp = M->p ;
    const int8_t  *GB_RESTRICT Mb = M->b ;
    const int64_t *GB_RESTRICT Mh = M->h ;
    const int64_t *GB_RESTRICT Mi = M->i ;
    const GB_void *GB_RESTRICT Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;
    const size_t mvlen = M->vlen ;

    GB_CTYPE *GB_RESTRICT Cx = (GB_CTYPE *) C->x ;
    const int64_t cvlen = C->vlen ;

    //--------------------------------------------------------------------------
    // C<M> = x
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        // if kfirst > klast then taskid does no work at all
        int64_t kfirst = kfirst_slice [taskid] ;
        int64_t klast  = klast_slice  [taskid] ;

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
                kfirst, klast, pstart_slice, Mp, mvlen) ;

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

