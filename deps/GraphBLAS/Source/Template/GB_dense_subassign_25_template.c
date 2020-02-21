//------------------------------------------------------------------------------
// GB_dense_subassign_25_template: C<M> = A where C is empty and A is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = A where C starts as empty, M is structural, and A is dense.  The
// pattern of C is an exact copy of M.

{

    //--------------------------------------------------------------------------
    // get C, M, and A
    //--------------------------------------------------------------------------

    GB_CTYPE *GB_RESTRICT Cx = C->x ;

    const int64_t *GB_RESTRICT Mp = M->p ;
    const int64_t *GB_RESTRICT Mh = M->h ;
    const int64_t *GB_RESTRICT Mi = M->i ;

    const GB_CTYPE *GB_RESTRICT Ax = A->x ;
    const int64_t avlen = A->vlen ;

    //--------------------------------------------------------------------------
    // C<M> = A
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        // if kfirst > klast then taskid does no work at all
        int64_t kfirst = kfirst_slice [taskid] ;
        int64_t klast  = klast_slice  [taskid] ;

        //----------------------------------------------------------------------
        // C<M(:,kfirst:klast)> = A(:,kfirst:klast)
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

            // pA points to the start of A(:,j) since A is dense
            int64_t pA = j * avlen ;

            //------------------------------------------------------------------
            // C<M(:,j)> = A(:,j)
            //------------------------------------------------------------------

            GB_PRAGMA_VECTORIZE
            for (int64_t pM = pM_start ; pM < pM_end ; pM++)
            { 
                int64_t p = pA + Mi [pM] ;
                GB_COPY_A_TO_C (Cx, pM, Ax, p) ;    // Cx [pM] = Ax [p]
            }
        }
    }
}

