//------------------------------------------------------------------------------
// GB_AxB_dot2_mask:  C<M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

{

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) collapse(2)
    for (int a_taskid = 0 ; a_taskid < naslice ; a_taskid++)
    for (int b_taskid = 0 ; b_taskid < nbslice ; b_taskid++)
    {

        //----------------------------------------------------------------------
        // get A
        //----------------------------------------------------------------------

        GrB_Matrix A = Aslice [a_taskid] ;

        #if defined ( GB_PHASE_1_OF_2 )
        int64_t *restrict C_count = C_counts [a_taskid] ;
        #else
        int64_t *restrict C_count_start =
            (a_taskid == 0) ?         NULL : C_counts [a_taskid] ;
        int64_t *restrict C_count_end   =
            (a_taskid == naslice-1) ? NULL : C_counts [a_taskid+1] ;
        const GB_ATYPE *restrict Ax = A_is_pattern ? NULL : A->x ;
        #endif

        const int64_t *restrict Ah = A->h ;
        const int64_t *restrict Ap = A->p ;
        const int64_t *restrict Ai = A->i ;
        int64_t anvec = A->nvec ;
        bool A_is_hyper = GB_IS_HYPER (A) ;

        //----------------------------------------------------------------------
        // get first and last non-empty vector of A
        //----------------------------------------------------------------------

        int64_t ia_first = -1, ia_last = -1 ;
        if (A_is_hyper)
        {
            // A is hypersparse or hyperslice
            if (anvec > 0)
            { 
                ia_first = Ah [0] ;
                ia_last  = Ah [anvec-1] ;
            }
        }
        else
        { 
            // A is standard sparse, or a slice.  For a standard matrix,
            // A->hfirst is zero and A->nvec = A->vdim, so ia_first and ia_last
            // include the whole matrix.
            ia_first = A->hfirst ;
            ia_last  = A->hfirst + anvec - 1 ;
        }

        bool A_is_slice    = ( A->is_slice && !A_is_hyper) ;
        bool A_is_standard = (!A->is_slice && !A_is_hyper) ;
        bool M_and_B_are_aliased = (M == B) ;

        //----------------------------------------------------------------------
        // C<M>=A'*B via dot products
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
            int64_t pM, pM_end, mjnz ;
            if (M_and_B_are_aliased)
            {
                pM = pB_start ;
                pM_end = pB_end ;
                mjnz = bjnz ;
            }
            else
            {
                int64_t mpleft = 0 ;
                GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mnvec-1, j,
                    &pM, &pM_end) ;
                // no work to do if M(:,j) is empty
                mjnz = pM_end - pM ;
                if (mjnz == 0) continue ;
            }

            //------------------------------------------------------------------
            // C(:,j)<M(:,j)> = A'*B(:,j)
            //------------------------------------------------------------------

            // get the first and last index in B(:,j)
            int64_t ib_first = Bi [pB_start] ;
            int64_t ib_last  = Bi [pB_end-1] ;

            if (A_is_standard)
            {

                //--------------------------------------------------------------
                // A is a standard sparse matrix
                //--------------------------------------------------------------

                // iterate over all entries in M(:,j)
                for ( ; pM < pM_end ; pM++)
                {

                    // get the next entry M(i,j)
                    int64_t i = Mi [pM] ;

                    // get the value of M(i,j) and skip if false
                    bool mij ;
                    cast_M (&mij, Mx +(pM*msize), 0) ;
                    if (!mij) continue ;

                    // get A(:,i), if it exists
                    int64_t pA     = Ap [i]   ;
                    int64_t pA_end = Ap [i+1] ;

                    // C(i,j) = A(:,i)'*B(:,j)
                    #include "GB_AxB_dot_cij.c"
                }

            }
            else if (mjnz <= anvec)
            {

                //--------------------------------------------------------------
                // M(:,j) is sparser than the vectors of A 
                //--------------------------------------------------------------

                // get the first and last index in M(:,j)
                int64_t im_first = Mi [pM] ;
                int64_t im_last  = Mi [pM_end-1] ;

                // no work to do if M(:,j) does not include any vectors in A
                if (ia_last < im_first || im_last < ia_first) continue ;

                // advance pM to the first vector of A
                if (im_first < ia_first)
                {
                    // search M(:,j) for the first vector of A
                    int64_t pright = pM_end - 1 ;
                    GB_BINARY_TRIM_SEARCH (ia_first, Mi, pM, pright) ;
                }

                int64_t pleft = 0 ;
                int64_t pright = anvec-1 ;

                // iterate over all entries in M(:,j)
                for ( ; pM < pM_end ; pM++)
                {

                    // get the next entry M(i,j)
                    int64_t i = Mi [pM] ;
                    if (i > ia_last)
                    {
                        // i is past last vector of A so the remainder of
                        // M(:,j) can be ignored
                        break ;
                    }

                    // the binary_trim_search of M(:,j), above, has trimmed the
                    // leading part of M(:,j), so i >= ia_first must hold here.
                    // The break statement above ensures that i <= ia_last holds
                    // here.  So M(i,j) exists, and i is in the range of the
                    // vectors of A
                    ASSERT (i >= ia_first && i <= ia_last) ;

                    // get the value of M(i,j) and skip if false
                    bool mij ;
                    cast_M (&mij, Mx +(pM*msize), 0) ;
                    if (!mij) continue ;

                    // get A(:,i), if it exists
                    int64_t pA, pA_end ;
                    if (A_is_slice)
                    {
                        // A is a slice
                        int64_t ka = i - ia_first ;
                        ASSERT (ka >= 0 && ka < anvec) ;
                        pA     = Ap [ka] ;
                        pA_end = Ap [ka+1] ;
                    }
                    else
                    {
                        // A is sparse, hypersparse, or hyperslice
                        GB_lookup (A_is_hyper, Ah, Ap, &pleft, pright, i,
                            &pA, &pA_end) ;
                    }

                    // C(i,j) = A(:,i)'*B(:,j)
                    #include "GB_AxB_dot_cij.c"
                }

            }
            else
            {

                //--------------------------------------------------------------
                // M(:,j) is denser than the vectors of A 
                //--------------------------------------------------------------

                // for each vector A(:,i):
                GBI_for_each_vector_with_iter (Iter_A, A)
                {
                    GBI_jth_iteration_with_iter (Iter_A, i, pA, pA_end) ;

                    // A(:,i) and B(:,j) are both present.  Check M(i,j).
                    bool mij = false ;
                    bool found ;
                    int64_t pright = pM_end - 1 ;
                    GB_BINARY_SEARCH (i, Mi, pM, pright, found) ;
                    if (found)
                    {
                        cast_M (&mij, Mx +(pM*msize), 0) ;
                    }
                    if (mij)
                    { 
                        // C(i,j) = A(:,i)'*B(:,j)
                        #include "GB_AxB_dot_cij.c"
                    }
                }
            }
        }
    }
}
