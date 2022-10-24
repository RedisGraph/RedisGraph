//------------------------------------------------------------------------------
// GB_emult_template: C=A.*B, C<M or !M>=A.*B when C is sparse/hyper
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Computes C=A.*B, C<M>=A.*B, or C<!M>=A.*B when C is sparse or hypersparse:

// phase1: does not compute C itself, but just counts the # of entries in each
// vector of C.  Fine tasks compute the # of entries in their slice of a
// single vector of C, and the results are cumsum'd.

// phase2: computes C, using the counts computed by phase1.

// No input matrix can be jumbled, and C is constructed as unjumbled.

// The following cases are handled:

        //      ------------------------------------------
        //      C       =           A       .*      B
        //      ------------------------------------------
        //      sparse  .           sparse          sparse  (method: 8)

        //      ------------------------------------------
        //      C       <M>=        A       .*      B
        //      ------------------------------------------
        //      sparse  sparse      sparse          sparse  (method: 8)
        //      sparse  bitmap      sparse          sparse  (method: 8)
        //      sparse  full        sparse          sparse  (method: 8)
        //      sparse  sparse      sparse          bitmap  (9 or 2)
        //      sparse  sparse      sparse          full    (9 or 2)
        //      sparse  sparse      bitmap          sparse  (10 or 3)
        //      sparse  sparse      full            sparse  (10 or 3)

        //      ------------------------------------------
        //      C       <!M>=       A       .*      B
        //      ------------------------------------------
        //      sparse  sparse      sparse          sparse  (8: M later)
        //      sparse  bitmap      sparse          sparse  (method: 8)
        //      sparse  full        sparse          sparse  (method: 8)

// Methods 9 and 10 are not yet implemented, and are currently handled by this
// Method 8 instead.  those cases.  Methods 2 and 3 can be used as well, but
// only if M is applied later.  See GB_emult_sparsity for this decision.

{

    int taskid ;
    #pragma omp parallel for num_threads(C_nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < C_ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        int64_t kfirst = TaskList [taskid].kfirst ;
        int64_t klast  = TaskList [taskid].klast ;
        bool fine_task = (klast == -1) ;
        int64_t len ;
        if (fine_task)
        { 
            // a fine task operates on a slice of a single vector
            klast = kfirst ;
            len = TaskList [taskid].len ;
        }
        else
        { 
            // a coarse task operates on one or more whole vectors
            len = vlen ;
        }

        //----------------------------------------------------------------------
        // compute all vectors in this task
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // get j, the kth vector of C
            //------------------------------------------------------------------

            int64_t j = GBH (Ch, k) ;

            #if defined ( GB_PHASE_1_OF_2 )
            int64_t cjnz = 0 ;
            #else
            int64_t pC, pC_end ;
            if (fine_task)
            { 
                // A fine task computes a slice of C(:,j)
                pC     = TaskList [taskid  ].pC ;
                pC_end = TaskList [taskid+1].pC ;
                ASSERT (Cp [k] <= pC && pC <= pC_end && pC_end <= Cp [k+1]) ;
            }
            else
            { 
                // The vectors of C are never sliced for a coarse task.
                pC     = Cp [k] ;
                pC_end = Cp [k+1] ;
            }
            int64_t cjnz = pC_end - pC ;
            if (cjnz == 0) continue ;
            #endif

            //------------------------------------------------------------------
            // get A(:,j)
            //------------------------------------------------------------------

            int64_t pA = -1, pA_end = -1 ;
            if (fine_task)
            { 
                // A fine task operates on Ai,Ax [pA...pA_end-1], which is
                // a subset of the vector A(:,j)
                pA     = TaskList [taskid].pA ;
                pA_end = TaskList [taskid].pA_end ;
            }
            else
            {
                // A coarse task operates on the entire vector A (:,j)
                int64_t kA = (Ch == Ah) ? k :
                            ((C_to_A == NULL) ? j : C_to_A [k]) ;
                if (kA >= 0)
                { 
                    pA     = GBP (Ap, kA, vlen) ;
                    pA_end = GBP (Ap, kA+1, vlen) ;
                }
            }

            int64_t ajnz = pA_end - pA ;        // nnz in A(:,j) for this slice
            int64_t pA_start = pA ;
            bool adense = (ajnz == len) ;

            // get the first and last indices in A(:,j) for this vector
            int64_t iA_first = -1 ;
            if (ajnz > 0)
            { 
                iA_first = GBI (Ai, pA, vlen) ;
            }
            #if defined ( GB_PHASE_1_OF_2 ) || defined ( GB_DEBUG )
            int64_t iA_last = -1 ;
            if (ajnz > 0)
            { 
                iA_last  = GBI (Ai, pA_end-1, vlen) ;
            }
            #endif

            //------------------------------------------------------------------
            // get B(:,j)
            //------------------------------------------------------------------

            int64_t pB = -1, pB_end = -1 ;
            if (fine_task)
            { 
                // A fine task operates on Bi,Bx [pB...pB_end-1], which is
                // a subset of the vector B(:,j)
                pB     = TaskList [taskid].pB ;
                pB_end = TaskList [taskid].pB_end ;
            }
            else
            {
                // A coarse task operates on the entire vector B (:,j)
                int64_t kB = (Ch == Bh) ? k :
                            ((C_to_B == NULL) ? j : C_to_B [k]) ;
                if (kB >= 0)
                { 
                    pB     = GBP (Bp, kB, vlen) ;
                    pB_end = GBP (Bp, kB+1, vlen) ;
                }
            }

            int64_t bjnz = pB_end - pB ;        // nnz in B(:,j) for this slice
            int64_t pB_start = pB ;
            bool bdense = (bjnz == len) ;

            // get the first and last indices in B(:,j) for this vector
            int64_t iB_first = -1 ;
            if (bjnz > 0)
            { 
                iB_first = GBI (Bi, pB, vlen) ;
            }
            #if defined ( GB_PHASE_1_OF_2 ) || defined ( GB_DEBUG )
            int64_t iB_last = -1 ;
            if (bjnz > 0)
            { 
                iB_last  = GBI (Bi, pB_end-1, vlen) ;
            }
            #endif

            //------------------------------------------------------------------
            // get M(:,j) if M is sparse or hypersparse
            //------------------------------------------------------------------

            int64_t pM = -1 ;
            int64_t pM_end = -1 ;
            if (M_is_sparse_or_hyper)
            {
                if (fine_task)
                { 
                    // A fine task operates on Mi,Mx [pM...pM_end-1], which is
                    // a subset of the vector M(:,j)
                    pM     = TaskList [taskid].pM ;
                    pM_end = TaskList [taskid].pM_end ;
                }
                else
                {
                    int64_t kM = -1 ;
                    if (Ch == Mh)
                    { 
                        // Ch is the same as Mh (a shallow copy), or both NULL
                        kM = k ;
                    }
                    else
                    { 
                        kM = (C_to_M == NULL) ? j : C_to_M [k] ;
                    }
                    if (kM >= 0)
                    { 
                        pM     = GBP (Mp, kM, vlen) ;
                        pM_end = GBP (Mp, kM+1, vlen) ;
                    }
                }
            }

            //------------------------------------------------------------------
            // C(:,j)<optional mask> = A (:,j) .* B (:,j) or subvector
            //------------------------------------------------------------------

            #if defined ( GB_PHASE_1_OF_2 )

            if (ajnz == 0 || bjnz == 0)
            { 

                //--------------------------------------------------------------
                // Method8(a): A(:,j) and/or B(:,j) are empty
                //--------------------------------------------------------------

                ;

            }
            else if (iA_last < iB_first || iB_last < iA_first)
            { 

                //--------------------------------------------------------------
                // Method8(a): intersection of A(:,j) and B(:,j) is empty
                //--------------------------------------------------------------

                // the last entry of A(:,j) comes before the first entry
                // of B(:,j), or visa versa
                ;

            }
            else

            #endif

            if (M == NULL)
            {

                //--------------------------------------------------------------
                // Method8(b,c,d): C = A.*B, no mask
                //--------------------------------------------------------------

                //      ------------------------------------------
                //      C       =           A       .*      B
                //      ------------------------------------------
                //      sparse  .           sparse          sparse  (method: 8)
                //      sparse  sparse      sparse          sparse  (8, M later)

                // both A and B are sparse/hyper
                ASSERT (A_is_sparse || A_is_hyper) ;
                ASSERT (B_is_sparse || B_is_hyper) ;

                if (ajnz > 32 * bjnz)
                {

                    //----------------------------------------------------------
                    // Method8(b): A(:,j) is much denser than B(:,j)
                    //----------------------------------------------------------

                    for ( ; pB < pB_end ; pB++)
                    {
                        int64_t i = Bi [pB] ;
                        // find i in A(:,j)
                        int64_t pright = pA_end - 1 ;
                        bool found ;
                        GB_BINARY_SEARCH (i, Ai, pA, pright, found) ;
                        if (found)
                        { 
                            // C (i,j) = A (i,j) .* B (i,j)
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            ASSERT (pC < pC_end) ;
                            Ci [pC] = i ;
                            #ifndef GB_ISO_EMULT
                            GB_GETA (aij, Ax, pA, A_iso) ;
                            GB_GETB (bij, Bx, pB, B_iso) ;
                            GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                            #endif
                            pC++ ;
                            #endif
                        }
                    }
                    #if defined ( GB_PHASE_2_OF_2 )
                    ASSERT (pC == pC_end) ;
                    #endif

                }
                else if (bjnz > 32 * ajnz)
                {

                    //----------------------------------------------------------
                    // Method8(c): B(:,j) is much denser than A(:,j)
                    //----------------------------------------------------------

                    for ( ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;
                        // find i in B(:,j)
                        int64_t pright = pB_end - 1 ;
                        bool found ;
                        GB_BINARY_SEARCH (i, Bi, pB, pright, found) ;
                        if (found)
                        { 
                            // C (i,j) = A (i,j) .* B (i,j)
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            ASSERT (pC < pC_end) ;
                            Ci [pC] = i ;
                            #ifndef GB_ISO_EMULT
                            GB_GETA (aij, Ax, pA, A_iso) ;
                            GB_GETB (bij, Bx, pB, B_iso) ;
                            GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                            #endif
                            pC++ ;
                            #endif
                        }
                    }
                    #if defined ( GB_PHASE_2_OF_2 )
                    ASSERT (pC == pC_end) ;
                    #endif

                }
                else
                {

                    //----------------------------------------------------------
                    // Method8(d): A(:,j) and B(:,j) about the sparsity
                    //----------------------------------------------------------

                    // linear-time scan of A(:,j) and B(:,j)

                    while (pA < pA_end && pB < pB_end)
                    {
                        int64_t iA = Ai [pA] ;
                        int64_t iB = Bi [pB] ;
                        if (iA < iB)
                        { 
                            // A(i,j) exists but not B(i,j)
                            pA++ ;
                        }
                        else if (iB < iA)
                        { 
                            // B(i,j) exists but not A(i,j)
                            pB++ ;
                        }
                        else
                        { 
                            // both A(i,j) and B(i,j) exist
                            // C (i,j) = A (i,j) .* B (i,j)
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            ASSERT (pC < pC_end) ;
                            Ci [pC] = iB ;
                            #ifndef GB_ISO_EMULT
                            GB_GETA (aij, Ax, pA, A_iso) ;
                            GB_GETB (bij, Bx, pB, B_iso) ;
                            GB_BINOP (GB_CX (pC), aij, bij, iB, j) ;
                            #endif
                            pC++ ;
                            #endif
                            pA++ ;
                            pB++ ;
                        }
                    }

                    #if defined ( GB_PHASE_2_OF_2 )
                    ASSERT (pC == pC_end) ;
                    #endif
                }

            }
            else if (M_is_sparse_or_hyper)
            {

                //--------------------------------------------------------------
                // Method8(e): C and M are sparse or hypersparse
                //--------------------------------------------------------------

                //      ------------------------------------------
                //      C       <M>=        A       .*      B
                //      ------------------------------------------
                //      sparse  sparse      sparse          sparse  (method: 8)
                //      sparse  sparse      sparse          bitmap  (9 or 2)
                //      sparse  sparse      sparse          full    (9 or 2)
                //      sparse  sparse      bitmap          sparse  (10 or 3)
                //      sparse  sparse      full            sparse  (10 or 3)

                // Methods 9 and 10 are not yet implemented; using Method 8
                // (GB_emult_phase[123]) instead.

                // ether A or B are sparse/hyper
                ASSERT (A_is_sparse || A_is_hyper || B_is_sparse || B_is_hyper);

                for ( ; pM < pM_end ; pM++)
                {

                    //----------------------------------------------------------
                    // get M(i,j) for A(i,j) .* B (i,j)
                    //----------------------------------------------------------

                    int64_t i = GBI (Mi, pM, vlen) ;
                    bool mij = GB_mcast (Mx, pM, msize) ;
                    if (!mij) continue ;

                    //----------------------------------------------------------
                    // get A(i,j)
                    //----------------------------------------------------------

                    bool afound ;
                    if (adense)
                    { 
                        // A(:,j) is dense, bitmap, or full; use quick lookup
                        pA = pA_start + i - iA_first ;
                        afound = GBB (Ab, pA) ;
                    }
                    else
                    { 
                        // A(:,j) is sparse; use binary search for A(i,j)
                        int64_t apright = pA_end - 1 ;
                        GB_BINARY_SEARCH (i, Ai, pA, apright, afound) ;
                    }
                    if (!afound) continue ;
                    ASSERT (GBI (Ai, pA, vlen) == i) ;

                    //----------------------------------------------------------
                    // get B(i,j)
                    //----------------------------------------------------------

                    bool bfound ;
                    if (bdense)
                    { 
                        // B(:,j) is dense; use direct lookup for B(i,j)
                        pB = pB_start + i - iB_first ;
                        bfound = GBB (Bb, pB) ;
                    }
                    else
                    { 
                        // B(:,j) is sparse; use binary search for B(i,j)
                        int64_t bpright = pB_end - 1 ;
                        GB_BINARY_SEARCH (i, Bi, pB, bpright, bfound) ;
                    }
                    if (!bfound) continue ;
                    ASSERT (GBI (Bi, pB, vlen) == i) ;

                    //----------------------------------------------------------
                    // C(i,j) = A(i,j) .* B(i,j)
                    //----------------------------------------------------------

                    // C (i,j) = A (i,j) .* B (i,j)
                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz++ ;
                    #else
                    Ci [pC] = i ;
                    #ifndef GB_ISO_EMULT
                    GB_GETA (aij, Ax, pA, A_iso) ;
                    GB_GETB (bij, Bx, pB, B_iso) ;
                    GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                    #endif
                    pC++ ;
                    #endif
                }

                #if defined ( GB_PHASE_2_OF_2 )
                ASSERT (pC == pC_end) ;
                #endif

            }
            else
            {

                //--------------------------------------------------------------
                // M is bitmap or full, for either C<M>=A.*B or C<!M>=A.*B
                //--------------------------------------------------------------

                //      ------------------------------------------
                //      C       <M>=        A       .*      B
                //      ------------------------------------------
                //      sparse  bitmap      sparse          sparse  (method: 8)
                //      sparse  full        sparse          sparse  (method: 8)

                //      ------------------------------------------
                //      C       <!M>=       A       .*      B
                //      ------------------------------------------
                //      sparse  bitmap      sparse          sparse  (method: 8)
                //      sparse  full        sparse          sparse  (method: 8)

                // GB_GET_MIJ: get M(i,j) where M is bitmap or full
                #undef  GB_GET_MIJ
                #define GB_GET_MIJ(i)                                     \
                    int64_t pM = pM_start + i ;                           \
                    bool mij = GBB (Mb, pM) && GB_mcast (Mx, pM, msize) ; \
                    if (Mask_comp) mij = !mij ;

                // both A and B are sparse/hyper
                ASSERT (A_is_sparse || A_is_hyper) ;
                ASSERT (B_is_sparse || B_is_hyper) ;

                int64_t pM_start = j * vlen ;

                if (ajnz > 32 * bjnz)
                {

                    //----------------------------------------------------------
                    // Method8(f): A(:,j) much denser than B(:,j), M bitmap/full
                    //----------------------------------------------------------

                    for ( ; pB < pB_end ; pB++)
                    {
                        int64_t i = Bi [pB] ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        {
                            // find i in A(:,j)
                            int64_t pright = pA_end - 1 ;
                            bool found ;
                            GB_BINARY_SEARCH (i, Ai, pA, pright, found) ;
                            if (found)
                            { 
                                // C (i,j) = A (i,j) .* B (i,j)
                                #if defined ( GB_PHASE_1_OF_2 )
                                cjnz++ ;
                                #else
                                ASSERT (pC < pC_end) ;
                                Ci [pC] = i ;
                                #ifndef GB_ISO_EMULT
                                GB_GETA (aij, Ax, pA, A_iso) ;
                                GB_GETB (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                                #endif
                                pC++ ;
                                #endif
                            }
                        }
                    }

                    #if defined ( GB_PHASE_2_OF_2 )
                    ASSERT (pC == pC_end) ;
                    #endif

                }
                else if (bjnz > 32 * ajnz)
                {

                    //----------------------------------------------------------
                    // Method8(g): B(:,j) much denser than A(:,j), M bitmap/full
                    //----------------------------------------------------------

                    for ( ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        {

                            // find i in B(:,j)
                            int64_t pright = pB_end - 1 ;
                            bool found ;
                            GB_BINARY_SEARCH (i, Bi, pB, pright, found) ;
                            if (found)
                            { 
                                // C (i,j) = A (i,j) .* B (i,j)
                                #if defined ( GB_PHASE_1_OF_2 )
                                cjnz++ ;
                                #else
                                ASSERT (pC < pC_end) ;
                                Ci [pC] = i ;
                                #ifndef GB_ISO_EMULT
                                GB_GETA (aij, Ax, pA, A_iso) ;
                                GB_GETB (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                                #endif
                                pC++ ;
                                #endif
                            }
                        }
                    }

                    #if defined ( GB_PHASE_2_OF_2 )
                    ASSERT (pC == pC_end) ;
                    #endif

                }
                else
                {

                    //----------------------------------------------------------
                    // Method8(h): A(:,j) and B(:,j) about same, M bitmap/full
                    //----------------------------------------------------------

                    // linear-time scan of A(:,j) and B(:,j)

                    while (pA < pA_end && pB < pB_end)
                    {
                        int64_t iA = Ai [pA] ;
                        int64_t iB = Bi [pB] ;
                        if (iA < iB)
                        { 
                            // A(i,j) exists but not B(i,j)
                            pA++ ;
                        }
                        else if (iB < iA)
                        { 
                            // B(i,j) exists but not A(i,j)
                            pB++ ;
                        }
                        else
                        {
                            // both A(i,j) and B(i,j) exist
                            int64_t i = iA ;
                            GB_GET_MIJ (i) ;
                            if (mij)
                            { 
                                // C (i,j) = A (i,j) .* B (i,j)
                                #if defined ( GB_PHASE_1_OF_2 )
                                cjnz++ ;
                                #else
                                ASSERT (pC < pC_end) ;
                                Ci [pC] = i ;
                                #ifndef GB_ISO_EMULT
                                GB_GETA (aij, Ax, pA, A_iso) ;
                                GB_GETB (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), aij, bij, iB, j) ;
                                #endif
                                pC++ ;
                                #endif
                            }
                            pA++ ;
                            pB++ ;
                        }
                    }

                    #if defined ( GB_PHASE_2_OF_2 )
                    ASSERT (pC == pC_end) ;
                    #endif
                }
            }

            //------------------------------------------------------------------
            // final count of nnz (C (:,j))
            //------------------------------------------------------------------

            #if defined ( GB_PHASE_1_OF_2 )
            if (fine_task)
            { 
                TaskList [taskid].pC = cjnz ;
            }
            else
            { 
                Cp [k] = cjnz ;
            }
            #endif
        }
    }
}

