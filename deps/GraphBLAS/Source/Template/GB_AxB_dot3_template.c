//------------------------------------------------------------------------------
// GB_AxB_dot3_template: C<M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_DOT3
#define GB_DOT3
#endif

{

    //--------------------------------------------------------------------------
    // get M, A, B, and C
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Cp = C->p ;
    const int64_t *GB_RESTRICT Ch = C->h ;
    int64_t  *GB_RESTRICT Ci = C->i ;
    GB_CTYPE *GB_RESTRICT Cx = C->x ;

    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bh = B->h ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    const GB_BTYPE *GB_RESTRICT Bx = B_is_pattern ? NULL : B->x ;
    const int64_t bvlen = B->vlen ;
    const int64_t bnvec = B->nvec ;
    const bool B_is_hyper = B->is_hyper ;

    const int64_t *GB_RESTRICT Mi = M->i ;
    const GB_void *GB_RESTRICT Mx = (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;

    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    const int64_t anvec = A->nvec ;
    const bool A_is_hyper = GB_IS_HYPER (A) ;
    const GB_ATYPE *GB_RESTRICT Ax = A_is_pattern ? NULL : A->x ;

    //--------------------------------------------------------------------------
    // C<M> = A'*B
    //--------------------------------------------------------------------------

    // C and M have the same pattern, except some entries of C may become
    // zombies.
    int64_t nzombies = 0 ;

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:nzombies)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        int64_t kfirst = TaskList [taskid].kfirst ;
        int64_t klast  = TaskList [taskid].klast ;
        int64_t pC_first = TaskList [taskid].pC ;
        int64_t pC_last  = TaskList [taskid].pC_end ;
        int64_t task_nzombies = 0 ;
        int64_t bpleft = 0 ;

        //----------------------------------------------------------------------
        // compute all vectors in this task
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // get C(:,k) and M(:k)
            //------------------------------------------------------------------

            int64_t j = (Ch == NULL) ? k : Ch [k] ;
            int64_t pC_start, pC_end ;
            if (k == kfirst)
            { 
                // First vector for task; may only be partially owned.
                pC_start = pC_first ;
                pC_end   = GB_IMIN (Cp [k+1], pC_last) ;
            }
            else if (k == klast)
            { 
                // Last vector for task; may only be partially owned.
                pC_start = Cp [k] ;
                pC_end   = pC_last ;
            }
            else
            { 
                // task fully owns this vector C(:,k).
                pC_start = Cp [k] ;
                pC_end   = Cp [k+1] ;
            }

            //------------------------------------------------------------------
            // get B(:,j)
            //------------------------------------------------------------------

            int64_t pB_start, pB_end ;
            GB_lookup (B_is_hyper, Bh, Bp, &bpleft, bnvec-1, j,
                &pB_start, &pB_end) ;
            int64_t bjnz = pB_end - pB_start ;

            //------------------------------------------------------------------
            // C(:,j)<M(:,j)> = A(:,i)'*B(:,j)
            //------------------------------------------------------------------

            if (bjnz == 0)
            {
            
                //--------------------------------------------------------------
                // C(:,j) is empty if B(:,j) is empty
                //--------------------------------------------------------------

                task_nzombies += (pC_end - pC_start) ;
                for (int64_t pC = pC_start ; pC < pC_end ; pC++)
                { 
                    // C(i,j) is a zombie
                    Ci [pC] = GB_FLIP (Mi [pC]) ;
                }
            }
            else
            {

                //--------------------------------------------------------------
                // B(:,j) not empty
                //--------------------------------------------------------------

                int64_t ib_first = Bi [pB_start] ;
                int64_t ib_last  = Bi [pB_end-1] ;
                int64_t apleft = 0 ;

                for (int64_t pC = pC_start ; pC < pC_end ; pC++)
                {

                    //----------------------------------------------------------
                    // compute C(i,j)
                    //----------------------------------------------------------

                    // get the value of M(i,j)
                    int64_t i = Mi [pC] ;
                    if (GB_mcast (Mx, pC, msize))   // note: Mx [pC], same as Cx
                    { 

                        //------------------------------------------------------
                        // M(i,j) is true, so compute C(i,j)
                        //------------------------------------------------------

                        // get A(:,i), if it exists
                        int64_t pA, pA_end ;
                        GB_lookup (A_is_hyper, Ah, Ap, &apleft, anvec-1, i,
                            &pA, &pA_end) ;

                        // C(i,j) = A(:,i)'*B(:,j)
                        #include "GB_AxB_dot_cij.c"
                    }
                    else
                    { 

                        //------------------------------------------------------
                        // M(i,j) is false, so C(i,j) is a zombie
                        //------------------------------------------------------

                        task_nzombies++ ;
                        Ci [pC] = GB_FLIP (i) ;
                    }
                }
            }
        }

        //----------------------------------------------------------------------
        // sum up the zombies found by this task
        //----------------------------------------------------------------------

        nzombies += task_nzombies ;
    }

    //--------------------------------------------------------------------------
    // finalize the zombie count for C
    //--------------------------------------------------------------------------

    C->nzombies = nzombies ;
}

#undef GB_DOT3
