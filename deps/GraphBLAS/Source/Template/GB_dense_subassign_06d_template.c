//------------------------------------------------------------------------------
// GB_dense_subassign_06d_template: C<A> = A where C is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get C and A
    //--------------------------------------------------------------------------

    const int64_t  *GB_RESTRICT Ap = A->p ;
    const int64_t  *GB_RESTRICT Ah = A->h ;
    const int64_t  *GB_RESTRICT Ai = A->i ;
    const GB_CTYPE *GB_RESTRICT Ax = A->x ;

    GB_CTYPE *GB_RESTRICT Cx = C->x ;
    const int64_t cvlen = C->vlen ;

    //--------------------------------------------------------------------------
    // C<A> = A
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        // if kfirst > klast then taskid does no work at all
        int64_t kfirst = kfirst_slice [taskid] ;
        int64_t klast  = klast_slice  [taskid] ;

        //----------------------------------------------------------------------
        // C<A(:,kfirst:klast)> = A(:,kfirst:klast)
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) to be operated on by this task
            //------------------------------------------------------------------

            int64_t j = (Ah == NULL) ? k : Ah [k] ;
            int64_t pA_start, pA_end ;
            GB_get_pA_and_pC (&pA_start, &pA_end, NULL,
                taskid, k, kfirst, klast, pstart_slice, NULL, NULL, Ap) ;

            // pC points to the start of C(:,j) if C is dense
            int64_t pC = j * cvlen ;

            //------------------------------------------------------------------
            // C<A(:,j)> = A(:,j)
            //------------------------------------------------------------------

            if (Mask_struct)
            {
                GB_PRAGMA_VECTORIZE
                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                { 
                    int64_t p = pC + Ai [pA] ;
                    GB_COPY_A_TO_C (Cx, p, Ax, pA) ;    // Cx [p] = Ax [pA]
                }
            }
            else
            {
                GB_PRAGMA_VECTORIZE
                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                {
                    if (GB_AX_MASK (Ax, pA, asize))
                    { 
                        int64_t p = pC + Ai [pA] ;
                        GB_COPY_A_TO_C (Cx, p, Ax, pA) ;    // Cx [p] = Ax [pA]
                    }
                }
            }
        }
    }
}

