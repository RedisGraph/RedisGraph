//------------------------------------------------------------------------------
// GB_AxB_dot2_compmask:  C<!M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{
    int ntasks = naslice * nbslice ;

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {
        int a_taskid = taskid / nbslice ;
        int b_taskid = taskid % nbslice ;

        //----------------------------------------------------------------------
        // get A
        //----------------------------------------------------------------------

        GrB_Matrix A = Aslice [a_taskid] ;
        const int64_t *GB_RESTRICT Ai = A->i ;

        #if defined ( GB_PHASE_1_OF_2 )
        int64_t *GB_RESTRICT C_count = C_counts [a_taskid] ;
        #else
        const int64_t *GB_RESTRICT C_count_start =
            (a_taskid == 0) ?         NULL : C_counts [a_taskid] ;
        const int64_t *GB_RESTRICT C_count_end   =
            (a_taskid == naslice-1) ? NULL : C_counts [a_taskid+1] ;
        const GB_ATYPE *GB_RESTRICT Ax = A_is_pattern ? NULL : A->x ;
        #endif

        //----------------------------------------------------------------------
        // C<!M>=A'*B via dot products
        //----------------------------------------------------------------------

        for (int64_t Iter_k = B_slice [b_taskid] ;
                     Iter_k < B_slice [b_taskid+1] ;
                     Iter_k++)
        {

            //------------------------------------------------------------------
            // get B(:,j)
            //------------------------------------------------------------------

            GBI_jth_iteration_with_iter (Iter, j, pB_start, pB_end) ;
            int64_t bjnz = pB_end - pB_start ;
            // no work to do if B(:,j) is empty
            if (bjnz == 0) continue ;

            //------------------------------------------------------------------
            // phase 2 of 2: get the range of entries in C(:,j) to compute
            //------------------------------------------------------------------

            #if defined ( GB_PHASE_2_OF_2 )
            // this thread computes Ci and Cx [cnz:cnz_last]
            int64_t cnz = Cp [Iter_k] +
                ((C_count_start == NULL) ? 0 : C_count_start [Iter_k]) ;
            int64_t cnz_last = (C_count_end == NULL) ?
                (Cp [Iter_k+1] - 1) :
                (Cp [Iter_k] + C_count_end [Iter_k] - 1) ;
            if (cnz > cnz_last) continue ;
            #endif

            //------------------------------------------------------------------
            // get M(:,j)
            //------------------------------------------------------------------

            // find vector j in M
            int64_t pM, pM_end ;
            int64_t mpleft = 0 ;
            GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mnvec-1, j, &pM, &pM_end) ;

            //------------------------------------------------------------------
            // C(:,j)<!M(:,j)> = A'*B(:,j)
            //------------------------------------------------------------------

            // get the first and last index in B(:,j)
            int64_t ib_first = Bi [pB_start] ;
            int64_t ib_last  = Bi [pB_end-1] ;

            // for each vector A(:,i):
            GBI_for_each_vector_with_iter (Iter_A, A)
            {
                GBI_jth_iteration_with_iter (Iter_A, i, pA, pA_end) ;

                // A(:,i) and B(:,j) are both present.  Check M(i,j).
                // FUTURE:: skip binary search if mask is dense.
                bool mij = false ;
                bool found ;
                int64_t pright = pM_end - 1 ;
                GB_BINARY_SEARCH (i, Mi, pM, pright, found) ;
                if (found)
                {
                    mij = GB_mcast (Mx, pM, msize) ;
                }
                if (!mij)
                { 
                    // C(i,j) = A(:,i)'*B(:,j)
                    #include "GB_AxB_dot_cij.c"
                }
            }
        }
    }
}

