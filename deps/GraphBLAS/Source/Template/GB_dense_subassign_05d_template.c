//------------------------------------------------------------------------------
// GB_dense_subassign_05d_template: C<M> = x where C is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get C and M
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Mp = M->p ;
    const int64_t *GB_RESTRICT Mh = M->h ;
    const int64_t *GB_RESTRICT Mi = M->i ;
    const GB_void *GB_RESTRICT Mx = (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;

    GB_CTYPE *GB_RESTRICT Cx = C->x ;
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

            int64_t j = (Mh == NULL) ? k : Mh [k] ;
            int64_t pM_start, pM_end ;
            GB_get_pA_and_pC (&pM_start, &pM_end, NULL,
                taskid, k, kfirst, klast, pstart_slice, NULL, NULL, Mp) ;

            // pC points to the start of C(:,j) if C is dense
            int64_t pC = j * cvlen ;

            //------------------------------------------------------------------
            // C<M(:,j)> = x
            //------------------------------------------------------------------

            if (Mx == NULL)
            {
                GB_PRAGMA_VECTORIZE
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                { 
                    int64_t p = pC + Mi [pM] ;
                    GB_COPY_SCALAR_TO_C (p, cwork) ;        // Cx [p] = scalar
                }
            }
            else
            {
                GB_PRAGMA_VECTORIZE
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                {
                    if (GB_mcast (Mx, pM, msize))
                    { 
                        int64_t p = pC + Mi [pM] ;
                        GB_COPY_SCALAR_TO_C (p, cwork) ;    // Cx [p] = scalar
                    }
                }
            }
        }
    }
}

