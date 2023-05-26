//------------------------------------------------------------------------------
// GB_sparse_add_template:  C=A+B, C<M>=A+B when C is sparse/hypersparse
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is sparse or hypersparse:

        //      ------------------------------------------
        //      C       =           A       +       B
        //      ------------------------------------------
        //      sparse  .           sparse          sparse

        //      ------------------------------------------
        //      C      <M> =        A       +       B
        //      ------------------------------------------
        //      sparse  sparse      sparse          sparse
        //      sparse  sparse      sparse          bitmap
        //      sparse  sparse      sparse          full
        //      sparse  sparse      bitmap          sparse
        //      sparse  sparse      bitmap          bitmap
        //      sparse  sparse      bitmap          full
        //      sparse  sparse      full            sparse
        //      sparse  sparse      full            bitmap
        //      sparse  sparse      full            full

        //      sparse  bitmap      sparse          sparse
        //      sparse  full        sparse          sparse

        //      ------------------------------------------
        //      C     <!M> =        A       +       B
        //      ------------------------------------------
        //      sparse  bitmap      sparse          sparse
        //      sparse  full        sparse          sparse

// If all four matrices are sparse/hypersparse, and C<!M>=A+B is being
// computed, then M is passed in as NULL to GB_add_phase*.  GB_add_sparsity
// returns apply_mask as false.  The methods below do not handle the case when
// C is sparse, M is sparse, and !M is used.  All other uses of !M when M
// is sparse result in a bitmap structure for C, and this is handled by
// GB_bitmap_add_template.

        // For this case: the mask is done later, so C=A+B is computed here:

        //      ------------------------------------------
        //      C     <!M> =        A       +       B
        //      ------------------------------------------
        //      sparse  sparse      sparse          sparse      (mask later)

{

    //--------------------------------------------------------------------------
    // phase1: count entries in each C(:,j)
    // phase2: compute C
    //--------------------------------------------------------------------------

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
                pC     = Cp [k  ] ;
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
                int64_t kA = (C_to_A == NULL) ? j : C_to_A [k] ;
                if (kA >= 0)
                { 
                    pA     = GBP (Ap, kA, vlen) ;
                    pA_end = GBP (Ap, kA+1, vlen) ;
                }
            }

            int64_t ajnz = pA_end - pA ;    // nnz in A(:,j) for this slice
            int64_t pA_start = pA ;
            bool adense = (ajnz == len) ;

            // get the first and last indices in A(:,j) for this vector
            int64_t iA_first = -1, iA_last = -1 ;
            if (ajnz > 0)
            { 
                iA_first = GBI (Ai, pA, vlen) ;
                iA_last  = GBI (Ai, pA_end-1, vlen) ;
            }

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
                int64_t kB = (C_to_B == NULL) ? j : C_to_B [k] ;
                if (kB >= 0)
                { 
                    pB     = GBP (Bp, kB, vlen) ;
                    pB_end = GBP (Bp, kB+1, vlen) ;
                }
            }

            int64_t bjnz = pB_end - pB ;    // nnz in B(:,j) for this slice
            int64_t pB_start = pB ;
            bool bdense = (bjnz == len) ;

            // get the first and last indices in B(:,j) for this vector
            int64_t iB_first = -1, iB_last = -1 ;
            if (bjnz > 0)
            { 
                iB_first = GBI (Bi, pB, vlen) ;
                iB_last  = GBI (Bi, pB_end-1, vlen) ;
            }

            //------------------------------------------------------------------
            // get M(:,j) if M is sparse or hypersparse
            //------------------------------------------------------------------

            bool sparse_mask_is_easy = false ;
            int64_t pM = -1 ;
            int64_t pM_end = -1 ;
            if (M_is_sparse_or_hyper)
            {
                if (fine_task)
                { 
                    // A fine task operates on Mi,Mx [pM...pM_end-1],
                    // which is a subset of the vector M(:,j)
                    pM     = TaskList [taskid].pM ;
                    pM_end = TaskList [taskid].pM_end ;
                }
                else
                {
                    int64_t kM = -1 ;
                    if (Ch_is_Mh)
                    { 
                        // Ch is the same as Mh (a deep copy)
                        ASSERT (Ch != NULL) ;
                        ASSERT (M_is_hyper) ;
                        ASSERT (Ch [k] == M->h [k]) ;
                        kM = k ;
                    }
                    else
                    { 
                        kM = (C_to_M == NULL) ? j : C_to_M [k] ;
                    }
                    if (kM >= 0)
                    { 
                        pM     = GBP (Mp, kM  , vlen) ;
                        pM_end = GBP (Mp, kM+1, vlen) ;
                    }
                }

                // The "easy mask" condition requires M to be sparse/hyper
                // and structural.  A and B cannot be bitmap.  Also one of
                // the following 3 conditions must hold:
                // (1) all entries are present in A(:,j) and B == M
                // (2) all entries are present in B(:,j) and A == M
                // (3) both A and B are aliased to M
                sparse_mask_is_easy =
                    Mask_struct &&          // M must be structural
                    !A_is_bitmap &&         // A must not be bitmap
                    !B_is_bitmap &&         // B must not be bitmap
                    ((adense && B == M) ||  // one of 3 conditions holds
                     (bdense && A == M) ||
                     (A == M && B == M)) ;

                // TODO: add the condition above to GB_add_sparsity,
                // where adense/bdense are true for the whole matrix
                // (adense is true if A is full, or sparse/hypersparse with
                // all entries present).  The test here is done vector by
                // vector, for each A(:,j) and B(:,j).  This is a finer grain
                // test, as compared to a test for all of A and B.

            }

            //------------------------------------------------------------------
            // C(:,j)<optional mask> = A (:,j) + B (:,j) or subvector
            //------------------------------------------------------------------

            if (M == NULL)
            {

                //--------------------------------------------------------------
                // M is not present, or !M is sparse but not applied here
                //--------------------------------------------------------------

                //      ------------------------------------------
                //      C       =           A       +       B
                //      ------------------------------------------
                //      sparse  .           sparse          sparse

                //      ------------------------------------------
                //      C     <!M> =        A       +       B
                //      ------------------------------------------
                //      sparse  sparse      sparse          sparse  (mask later)

                // If all four matrices are sparse or hypersparse, and
                // Mask_comp is true, the mask M is passed in to this method as
                // NULL.  C=A+B is computed with no mask, and !M is applied
                // later.

                // A and B are both sparse or hypersparse, not bitmap or
                // full, but individual vectors of A and B might have all
                // entries present (adense and/or bdense).
                ASSERT (A_is_sparse || A_is_hyper) ;
                ASSERT (B_is_sparse || B_is_hyper) ;

                #if defined ( GB_PHASE_1_OF_2 )

                if (A_and_B_are_disjoint)
                { 

                    // only used by GB_wait, which computes A+T where T is the
                    // matrix of pending tuples for A.  The pattern of pending
                    // tuples is always disjoint with the pattern of A.

                    cjnz = ajnz + bjnz ;

                }
                else

                #endif

                if (adense && bdense)
                {

                    //----------------------------------------------------------
                    // Method01: A(:,j) and B(:,j) dense: thus C(:,j) dense
                    //----------------------------------------------------------

                    ASSERT (ajnz == bjnz) ;
                    ASSERT (iA_first == iB_first) ;
                    ASSERT (iA_last  == iB_last ) ;
                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz ;
                    #else
                    ASSERT (cjnz == ajnz) ;
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        int64_t i = p + iA_first ;
                        Ci [pC + p] = i ;
                        ASSERT (Ai [pA + p] == i) ;
                        ASSERT (Bi [pB + p] == i) ;
                        #ifndef GB_ISO_ADD
                        GB_LOAD_A (aij, Ax, pA + p, A_iso) ;
                        GB_LOAD_B (bij, Bx, pB + p, B_iso) ;
                        GB_BINOP (GB_CX (pC + p), aij, bij, i, j) ;
                        #endif
                    }
                    #endif

                }
                else if (adense)
                {

                    //----------------------------------------------------------
                    // Method02: A(:,j) dense, B(:,j) sparse: C(:,j) dense
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz ;
                    #else
                    ASSERT (cjnz == ajnz) ;
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        int64_t i = p + iA_first ;
                        Ci [pC + p] = i ;
                        ASSERT (Ai [pA + p] == i) ;
                        #ifndef GB_ISO_ADD
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = A(i,j) + beta
                            GB_LOAD_A (aij, Ax, pA+p, A_iso) ;
                            GB_BINOP (GB_CX (pC+p), aij, beta_scalar, i, j) ;
                        }
                        #else
                        { 
                            // C (i,j) = A (i,j)
                            GB_COPY_A_TO_C (GB_CX (pC+p), Ax, pA+p, A_iso) ;
                        }
                        #endif
                        #endif
                    }
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        int64_t i = Bi [pB + p] ;
                        int64_t ii = i - iA_first ;
                        ASSERT (Ai [pA + ii] == i) ;
                        #ifndef GB_ISO_ADD
                        GB_LOAD_A (aij, Ax, pA + ii, A_iso) ;
                        GB_LOAD_B (bij, Bx, pB + p, B_iso) ;
                        GB_BINOP (GB_CX (pC + ii), aij, bij, i, j) ;
                        #endif
                    }
                    #endif

                }
                else if (bdense)
                {

                    //----------------------------------------------------------
                    // Method03: A(:,j) sparse, B(:,j) dense: C(:,j) dense
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = bjnz ;
                    #else
                    ASSERT (cjnz == bjnz) ;
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        int64_t i = p + iB_first ;
                        Ci [pC + p] = i ;
                        ASSERT (Bi [pB + p] == i) ;
                        #ifndef GB_ISO_ADD
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = alpha + B(i,j)
                            GB_LOAD_B (bij, Bx, pB+p, B_iso) ;
                            // GB_COMPILER_MSC_2019 workaround: the following
                            // line of code triggers a bug in the MSC 19.2x
                            // compiler in Visual Studio 2019, only for the
                            // FIRST_FC32 and SECOND_FC32 operators.  As a
                            // workaround, this template is not used for
                            // those operators when compiling GraphBLAS with
                            // this compiler.  Note the bug may also appear
                            // in VS2022, but this has not yet been tested.
                            GB_BINOP (GB_CX (pC+p), alpha_scalar, bij, i, j) ;
                        }
                        #else
                        { 
                            // C (i,j) = B (i,j)
                            GB_COPY_B_TO_C (GB_CX (pC+p), Bx, pB+p, B_iso) ;
                        }
                        #endif
                        #endif
                    }
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        int64_t i = Ai [pA + p] ;
                        int64_t ii = i - iB_first ;
                        ASSERT (Bi [pB + ii] == i) ;
                        #ifndef GB_ISO_ADD
                        GB_LOAD_A (aij, Ax, pA + p, A_iso) ;
                        GB_LOAD_B (bij, Bx, pB + ii, B_iso) ;
                        GB_BINOP (GB_CX (pC + ii), aij, bij, i, j) ;
                        #endif
                    }
                    #endif

                }
                else if (ajnz == 0)
                {

                    //----------------------------------------------------------
                    // Method04: A(:,j) is empty
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = bjnz ;
                    #else
                    ASSERT (cjnz == bjnz) ;
                    memcpy (Ci + pC, Bi + pB, bjnz * sizeof (int64_t)) ;
                    #ifndef GB_ISO_ADD
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = alpha + B(i,j)
                            GB_LOAD_B (bij, Bx, pB+p, B_iso) ;
                            GB_BINOP (GB_CX (pC+p), alpha_scalar, bij,
                                Bi [pB+p], j) ;
                        }
                        #else
                        { 
                            // C (i,j) = B (i,j)
                            GB_COPY_B_TO_C (GB_CX (pC+p), Bx, pB+p, B_iso) ;
                        }
                        #endif
                    }
                    #endif
                    #endif

                }
                else if (bjnz == 0)
                {

                    //----------------------------------------------------------
                    // Method05: B(:,j) is empty
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz ;
                    #else
                    ASSERT (cjnz == ajnz) ;
                    memcpy (Ci + pC, Ai + pA, ajnz * sizeof (int64_t)) ;
                    #ifndef GB_ISO_ADD
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = A(i,j) + beta
                            GB_LOAD_A (aij, Ax, pA+p, A_iso) ;
                            GB_BINOP (GB_CX (pC+p), aij, beta_scalar,
                                Ai [pA+p], j) ;
                        }
                        #else
                        { 
                            // C (i,j) = A (i,j)
                            GB_COPY_A_TO_C (GB_CX (pC+p), Ax, pA+p, A_iso) ;
                        }
                        #endif
                    }
                    #endif
                    #endif

                }
                else if (iA_last < iB_first)
                {

                    //----------------------------------------------------------
                    // Method06: last A(:,j) comes before 1st B(:,j)
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz + bjnz ;
                    #else
                    ASSERT (cjnz == ajnz + bjnz) ;
                    memcpy (Ci + pC, Ai + pA, ajnz * sizeof (int64_t)) ;
                    #ifndef GB_ISO_ADD
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = A(i,j) + beta
                            GB_LOAD_A (aij, Ax, pA+p, A_iso) ;
                            GB_BINOP (GB_CX (pC+p), aij, beta_scalar,
                                Ai [pA+p], j) ;
                        }
                        #else
                        { 
                            // C (i,j) = A (i,j)
                            GB_COPY_A_TO_C (GB_CX (pC+p), Ax, pA+p, A_iso) ;
                        }
                        #endif
                    }
                    #endif
                    pC += ajnz ;
                    memcpy (Ci + pC, Bi + pB, bjnz * sizeof (int64_t)) ;
                    #ifndef GB_ISO_ADD
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = alpha + B(i,j)
                            GB_LOAD_B (bij, Bx, pB+p, B_iso) ;
                            GB_BINOP (GB_CX (pC+p), alpha_scalar, bij,
                                Bi [pB+p], j) ;
                        }
                        #else
                        { 
                            // C (i,j) = B (i,j)
                            GB_COPY_B_TO_C (GB_CX (pC+p), Bx, pB+p, B_iso) ;
                        }
                        #endif
                    }
                    #endif
                    #endif

                }
                else if (iB_last < iA_first)
                {

                    //----------------------------------------------------------
                    // Method07: last B(:,j) comes before 1st A(:,j)
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz + bjnz ;
                    #else
                    ASSERT (cjnz == ajnz + bjnz) ;
                    memcpy (Ci + pC, Bi + pB, bjnz * sizeof (int64_t)) ;
                    #ifndef GB_ISO_ADD
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = alpha + B(i,j)
                            GB_LOAD_B (bij, Bx, pB+p, B_iso) ;
                            GB_BINOP (GB_CX (pC+p), alpha_scalar, bij,
                                Bi [pB+p], j) ;
                        }
                        #else
                        { 
                            // C (i,j) = B (i,j)
                            GB_COPY_B_TO_C (GB_CX (pC+p), Bx, pB+p, B_iso) ;
                        }
                        #endif
                    }
                    #endif
                    pC += bjnz ;
                    memcpy (Ci + pC, Ai + pA, ajnz * sizeof (int64_t)) ;
                    #ifndef GB_ISO_ADD
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = A(i,j) + beta
                            GB_LOAD_A (aij, Ax, pA+p, A_iso) ;
                            GB_BINOP (GB_CX (pC+p), aij, beta_scalar,
                                Ai [pA+p], j) ;
                        }
                        #else
                        { 
                            // C (i,j) = A (i,j)
                            GB_COPY_A_TO_C (GB_CX (pC+p), Ax, pA+p, A_iso) ;
                        }
                        #endif
                    }
                    #endif
                    #endif

                }

                #if defined ( GB_PHASE_1_OF_2 )
                else if (ajnz > 32 * bjnz)
                {

                    //----------------------------------------------------------
                    // Method08: A(:,j) is much denser than B(:,j)
                    //----------------------------------------------------------

                    // cjnz = ajnz + bjnz - nnz in the intersection

                    cjnz = ajnz + bjnz ;
                    for ( ; pB < pB_end ; pB++)
                    { 
                        int64_t i = Bi [pB] ;
                        // find i in A(:,j)
                        int64_t pright = pA_end - 1 ;
                        bool found ;
                        GB_BINARY_SEARCH (i, Ai, pA, pright, found) ;
                        if (found) cjnz-- ;
                    }

                }
                else if (bjnz > 32 * ajnz)
                {

                    //----------------------------------------------------------
                    // Method09: B(:,j) is much denser than A(:,j)
                    //----------------------------------------------------------

                    // cjnz = ajnz + bjnz - nnz in the intersection

                    cjnz = ajnz + bjnz ;
                    for ( ; pA < pA_end ; pA++)
                    { 
                        int64_t i = Ai [pA] ;
                        // find i in B(:,j)
                        int64_t pright = pB_end - 1 ;
                        bool found ;
                        GB_BINARY_SEARCH (i, Bi, pB, pright, found) ;
                        if (found) cjnz-- ;
                    }

                }
                #endif

                else
                {

                    //----------------------------------------------------------
                    // Method10: A(:,j) and B(:,j) about the same sparsity
                    //----------------------------------------------------------

                    while (pA < pA_end && pB < pB_end)
                    {
                        int64_t iA = Ai [pA] ;
                        int64_t iB = Bi [pB] ;
                        if (iA < iB)
                        { 
                            #if defined ( GB_PHASE_2_OF_2 )
                            Ci [pC] = iA ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (iA,j) = A(iA,j) + beta
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_BINOP (GB_CX (pC), aij, beta_scalar, iA, j) ;
                            }
                            #else
                            { 
                                // C (iA,j) = A (iA,j)
                                GB_COPY_A_TO_C (GB_CX (pC), Ax, pA, A_iso) ;
                            }
                            #endif
                            #endif
                            #endif
                            pA++ ;
                        }
                        else if (iA > iB)
                        { 
                            #if defined ( GB_PHASE_2_OF_2 )
                            Ci [pC] = iB ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (iB,j) = alpha + B(iB,j)
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), alpha_scalar, bij,
                                    iB, j) ;
                            }
                            #else
                            { 
                                // C (iB,j) = B (iB,j)
                                GB_COPY_B_TO_C (GB_CX (pC), Bx, pB, B_iso) ;
                            }
                            #endif
                            #endif
                            #endif
                            pB++ ;
                        }
                        else
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            #if defined ( GB_PHASE_2_OF_2 )
                            Ci [pC] = iB ;
                            #ifndef GB_ISO_ADD
                            GB_LOAD_A (aij, Ax, pA, A_iso) ;
                            GB_LOAD_B (bij, Bx, pB, B_iso) ;
                            GB_BINOP (GB_CX (pC), aij, bij, iB, j) ;
                            #endif
                            #endif
                            pA++ ;
                            pB++ ;
                        }
                        #if defined ( GB_PHASE_2_OF_2 )
                        pC++ ;
                        #else
                        cjnz++ ;
                        #endif
                    }

                    //----------------------------------------------------------
                    // A (:,j) or B (:,j) have entries left; not both
                    //----------------------------------------------------------

                    ajnz = (pA_end - pA) ;
                    bjnz = (pB_end - pB) ;
                    ASSERT (ajnz == 0 || bjnz == 0) ;
                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz += ajnz + bjnz ;
                    #else
                    memcpy (Ci + pC, Ai + pA, ajnz * sizeof (int64_t)) ;
                    #ifndef GB_ISO_ADD
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = A(i,j) + beta
                            GB_LOAD_A (aij, Ax, pA+p, A_iso) ;
                            GB_BINOP (GB_CX (pC+p), aij, beta_scalar,
                                Ai [pA+p], j) ;
                        }
                        #else
                        { 
                            // C (i,j) = A (i,j)
                            GB_COPY_A_TO_C (GB_CX (pC+p), Ax, pA+p, A_iso) ;
                        }
                        #endif
                    }
                    #endif
                    memcpy (Ci + pC, Bi + pB, bjnz * sizeof (int64_t)) ;
                    #ifndef GB_ISO_ADD
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = alpha + B(i,j)
                            GB_LOAD_B (bij, Bx, pB+p, B_iso) ;
                            GB_BINOP (GB_CX (pC+p), alpha_scalar, bij,
                                Bi [pB+p], j) ;
                        }
                        #else
                        { 
                            // C (i,j) = B (i,j)
                            GB_COPY_B_TO_C (GB_CX (pC+p), Bx, pB+p, B_iso) ;
                        }
                        #endif
                    }
                    #endif
                    ASSERT (pC + ajnz + bjnz == pC_end) ;
                    #endif
                }

            }
            else if (sparse_mask_is_easy)
            {

                //--------------------------------------------------------------
                // special case: M is present and very easy to use
                //--------------------------------------------------------------

                //      ------------------------------------------
                //      C      <M> =        A       +       B
                //      ------------------------------------------
                //      sparse  sparse      sparse          sparse
                //      sparse  sparse      sparse          full
                //      sparse  sparse      full            sparse
                //      sparse  sparse      full            full

                // A and B are sparse, hypersparse or full, not bitmap.
                ASSERT (!A_is_bitmap) ;
                ASSERT (!B_is_bitmap) ;
                ASSERT (Mask_struct) ;

                int64_t mjnz = pM_end - pM ;        // nnz (M (:,j))

                #if defined ( GB_PHASE_1_OF_2 )

                // M is structural, and sparse or hypersparse, so every entry
                // in the mask is guaranteed to appear in A+B.  The symbolic
                // count is thus trivial.

                cjnz = mjnz ;

                #else

                // copy the pattern into C (:,j)
                int64_t pC_start = pC ;
                int64_t pM_start = pM ;
                memcpy (Ci + pC, Mi + pM, mjnz * sizeof (int64_t)) ;
                int64_t pA_offset = pA_start - iA_first ;
                int64_t pB_offset = pB_start - iB_first ;

                if (adense && B == M)
                { 

                    //----------------------------------------------------------
                    // Method11: A dense, B == M
                    //----------------------------------------------------------

                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < mjnz ; p++)
                    {
                        int64_t pM = p + pM_start ;
                        int64_t pC = p + pC_start ;
                        int64_t i = Mi [pM] ;
                        ASSERT (GB_mcast (Mx, pM, msize)) ;
                        ASSERT (GBI (Ai, pA_offset + i, vlen) == i) ;
                        ASSERT (GBI (Bi, pM, vlen) == i) ;
                        #ifndef GB_ISO_ADD
                        GB_LOAD_A (aij, Ax, pA_offset + i, A_iso) ;
                        GB_LOAD_B (bij, Bx, pM, B_iso) ;
                        GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                        #endif
                    }

                }
                else if (bdense && A == M)
                { 

                    //----------------------------------------------------------
                    // Method12: B dense, A == M
                    //----------------------------------------------------------

                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < mjnz ; p++)
                    {
                        int64_t pM = p + pM_start ;
                        int64_t pC = p + pC_start ;
                        int64_t i = Mi [pM] ;
                        ASSERT (GB_mcast (Mx, pM, msize)) ;
                        ASSERT (GBI (Ai, pM, vlen) == i) ;
                        ASSERT (GBI (Bi, pB_offset + i, vlen) == i) ;
                        #ifndef GB_ISO_ADD
                        GB_LOAD_A (aij, Ax, pM, A_iso) ;
                        GB_LOAD_B (bij, Bx, pB_offset + i, B_iso) ;
                        GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                        #endif
                    }

                }
                else // (A == M) && (B == M)
                { 

                    //----------------------------------------------------------
                    // Method13: A == M == B: all three matrices the same
                    //----------------------------------------------------------

                    #ifndef GB_ISO_ADD
                    GB_PRAGMA_SIMD_VECTORIZE
                    for (int64_t p = 0 ; p < mjnz ; p++)
                    {
                        int64_t pM = p + pM_start ;
                        int64_t pC = p + pC_start ;
                        #if GB_OP_IS_SECOND
                        GB_LOAD_B (t, Bx, pM, B_iso) ;
                        #else
                        GB_LOAD_A (t, Ax, pM, A_iso) ;
                        #endif
                        GB_BINOP (GB_CX (pC), t, t, Mi [pM], j) ;
                    }
                    #endif

                }
                #endif

            }
            else if (M_is_sparse_or_hyper)
            {

                //--------------------------------------------------------------
                // Method14: C and M are sparse or hypersparse
                //--------------------------------------------------------------

                //      ------------------------------------------
                //      C      <M> =        A       +       B
                //      ------------------------------------------
                //      sparse  sparse      sparse          sparse  (*)
                //      sparse  sparse      sparse          bitmap  (*)
                //      sparse  sparse      sparse          full    (*)
                //      sparse  sparse      bitmap          sparse  (*)
                //      sparse  sparse      bitmap          bitmap  (+)
                //      sparse  sparse      bitmap          full    (+)
                //      sparse  sparse      full            sparse  (*)
                //      sparse  sparse      full            bitmap  (+)
                //      sparse  sparse      full            full    (+)

                // (*) This method is efficient except when either A or B are
                // sparse, and when M is sparse but with many entries.  When M
                // is sparse and either A or B are sparse, the method is
                // designed to be very efficient when M is very sparse compared
                // with A and/or B.  It traverses all entries in the sparse M,
                // and (for sparse A or B) does a binary search for entries in
                // A or B.  In that case, if M has many entries, the mask M
                // should be ignored, and C=A+B should be computed without any
                // mask.  The test for when to use M here should ignore A or B
                // if they are bitmap or full.

                // (+) TODO: if C and M are sparse/hyper, and A and B are
                // both bitmap/full, then use GB_emult_04_template instead,
                // but with (Ab [p] || Bb [p]) instead of (Ab [p] && Bb [p]).

                // A and B can have any sparsity pattern (hypersparse,
                // sparse, bitmap, or full).

                for ( ; pM < pM_end ; pM++)
                {

                    //----------------------------------------------------------
                    // get M(i,j) for A(i,j) + B (i,j)
                    //----------------------------------------------------------

                    int64_t i = Mi [pM] ;
                    bool mij = GB_mcast (Mx, pM, msize) ;
                    if (!mij) continue ;

                    //----------------------------------------------------------
                    // get A(i,j)
                    //----------------------------------------------------------

                    bool afound ;
                    if (adense)
                    { 
                        // A is dense, bitmap, or full; use quick lookup
                        pA = pA_start + (i - iA_first) ;
                        afound = GBB (Ab, pA) ;
                    }
                    else if (A == M)
                    { 
                        // A is aliased to M
                        pA = pM ;
                        afound = true ;
                    }
                    else
                    { 
                        // A is sparse; use binary search.  This is slow unless
                        // M is very sparse compared with A.
                        int64_t apright = pA_end - 1 ;
                        GB_BINARY_SEARCH (i, Ai, pA, apright, afound) ;
                    }

                    ASSERT (GB_IMPLIES (afound, GBI (Ai, pA, vlen) == i)) ;

                    //----------------------------------------------------------
                    // get B(i,j)
                    //----------------------------------------------------------

                    bool bfound ;
                    if (bdense)
                    { 
                        // B is dense; use quick lookup
                        pB = pB_start + (i - iB_first) ;
                        bfound = GBB (Bb, pB) ;
                    }
                    else if (B == M)
                    { 
                        // B is aliased to M
                        pB = pM ;
                        bfound = true ;
                    }
                    else
                    { 
                        // B is sparse; use binary search.  This is slow unless
                        // M is very sparse compared with B.
                        int64_t bpright = pB_end - 1 ;
                        GB_BINARY_SEARCH (i, Bi, pB, bpright, bfound) ;
                    }

                    ASSERT (GB_IMPLIES (bfound, GBI (Bi, pB, vlen) == i)) ;

                    //----------------------------------------------------------
                    // C(i,j) = A(i,j) + B(i,j)
                    //----------------------------------------------------------

                    if (afound && bfound)
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        #if defined ( GB_PHASE_1_OF_2 )
                        cjnz++ ;
                        #else
                        Ci [pC] = i ;
                        #ifndef GB_ISO_ADD
                        GB_LOAD_A (aij, Ax, pA, A_iso) ;
                        GB_LOAD_B (bij, Bx, pB, B_iso) ;
                        GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                        #endif
                        pC++ ;
                        #endif
                    }
                    else if (afound)
                    { 
                        #if defined ( GB_PHASE_1_OF_2 )
                        cjnz++ ;
                        #else
                        Ci [pC] = i ;
                        #ifndef GB_ISO_ADD
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = A(i,j) + beta
                            GB_LOAD_A (aij, Ax, pA, A_iso) ;
                            GB_BINOP (GB_CX (pC), aij, beta_scalar, i, j) ;
                        }
                        #else
                        { 
                            // C (i,j) = A (i,j)
                            GB_COPY_A_TO_C (GB_CX (pC), Ax, pA, A_iso) ;
                        }
                        #endif
                        #endif
                        pC++ ;
                        #endif
                    }
                    else if (bfound)
                    { 
                        #if defined ( GB_PHASE_1_OF_2 )
                        cjnz++ ;
                        #else
                        Ci [pC] = i ;
                        #ifndef GB_ISO_ADD
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = alpha + B(i,j)
                            GB_LOAD_B (bij, Bx, pB, B_iso) ;
                            GB_BINOP (GB_CX (pC), alpha_scalar, bij, i, j) ;
                        }
                        #else
                        { 
                            // C (i,j) = B (i,j)
                            GB_COPY_B_TO_C (GB_CX (pC), Bx, pB, B_iso) ;
                        }
                        #endif
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

                //--------------------------------------------------------------
                // M is bitmap or full, for either C<M>=A+B or C<!M>=A+B
                //--------------------------------------------------------------

                //      ------------------------------------------
                //      C      <M> =        A       +       B
                //      ------------------------------------------
                //      sparse  bitmap      sparse          sparse
                //      sparse  full        sparse          sparse

                //      ------------------------------------------
                //      C      <!M> =       A       +       B
                //      ------------------------------------------
                //      sparse  bitmap      sparse          sparse
                //      sparse  full        sparse          sparse

                // This method is very efficient for any mask, and should
                // always be used if M is bitmap or full, even if the mask must
                // also be applied later in GB_mask or GB_accum_mask.
                // Exploiting the mask here adds no extra search time, and it
                // reduces the size of C on output.

                // GB_GET_MIJ: get M(i,j) where M is bitmap or full
                #undef  GB_GET_MIJ
                #define GB_GET_MIJ(i)                                     \
                    int64_t pM = pM_start + i ;                           \
                    bool mij = GBB (Mb, pM) && GB_mcast (Mx, pM, msize) ; \
                    if (Mask_comp) mij = !mij ;

                // A and B are sparse or hypersparse, not bitmap or full,
                // but individual vectors of A and B might have all entries
                // present (adense and/or bdense).
                ASSERT (A_is_sparse || A_is_hyper) ;
                ASSERT (B_is_sparse || B_is_hyper) ;

                int64_t pM_start = j * vlen ;

                if (adense && bdense)
                {

                    //----------------------------------------------------------
                    // Method15: A(:,j) and B(:,j) dense, M bitmap/full
                    //----------------------------------------------------------

                    ASSERT (ajnz == bjnz) ;
                    ASSERT (iA_first == iB_first) ;
                    ASSERT (iA_last  == iB_last ) ;
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    {
                        int64_t i = p + iA_first ;
                        ASSERT (Ai [pA + p] == i) ;
                        ASSERT (Bi [pB + p] == i) ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = i ;
                            #ifndef GB_ISO_ADD
                            GB_LOAD_A (aij, Ax, pA + p, A_iso) ;
                            GB_LOAD_B (bij, Bx, pB + p, B_iso) ;
                            GB_BINOP (GB_CX (pC), aij, bij, i, j) ;
                            #endif
                            pC++ ;
                            #endif
                        }
                    }

                }
                else if (ajnz == 0)
                {

                    //----------------------------------------------------------
                    // Method16: A(:,j) is empty, M bitmap/full
                    //----------------------------------------------------------

                    for ( ; pB < pB_end ; pB++)
                    {
                        int64_t i = Bi [pB] ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        { 
                            // C (i,j) = B (i,j), or alpha + B(i,j)
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = i ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), alpha_scalar, bij, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (pC), Bx, pB, B_iso) ;
                            }
                            #endif
                            #endif
                            pC++ ;
                            #endif
                        }
                    }

                }
                else if (bjnz == 0)
                {

                    //----------------------------------------------------------
                    // Method17: B(:,j) is empty, M bitmap/full
                    //----------------------------------------------------------

                    for ( ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        { 
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = i ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_BINOP (GB_CX (pC), aij, beta_scalar, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (pC), Ax, pA, A_iso) ;
                            }
                            #endif
                            #endif
                            pC++ ;
                            #endif
                        }
                    }

                }
                else if (iA_last < iB_first)
                {

                    //----------------------------------------------------------
                    // Method18:last A(:,j) before 1st B(:,j), M bitmap/full
                    //----------------------------------------------------------

                    for ( ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        { 
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = i ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_BINOP (GB_CX (pC), aij, beta_scalar, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (pC), Ax, pA, A_iso) ;
                            }
                            #endif
                            #endif
                            pC++ ;
                            #endif
                        }
                    }

                    for ( ; pB < pB_end ; pB++)
                    {
                        int64_t i = Bi [pB] ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        { 
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = i ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), alpha_scalar, bij, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (pC), Bx, pB, B_iso) ;
                            }
                            #endif
                            #endif
                            pC++ ;
                            #endif
                        }
                    }

                }
                else if (iB_last < iA_first)
                {

                    //----------------------------------------------------------
                    // Method19:last B(:,j) before 1st A(:,j), M bitmap/full
                    //----------------------------------------------------------

                    for ( ; pB < pB_end ; pB++)
                    {
                        int64_t i = Bi [pB] ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        { 
                            // C (i,j) = B (i,j), or alpha + B(i,j)
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = i ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), alpha_scalar, bij, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (pC), Bx, pB, B_iso) ;
                            }
                            #endif
                            #endif
                            pC++ ;
                            #endif
                        }
                    }

                    for ( ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;
                        GB_GET_MIJ (i) ;
                        if (mij)
                        { 
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = i ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_BINOP (GB_CX (pC), aij, beta_scalar, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (pC), Ax, pA, A_iso) ;
                            }
                            #endif
                            #endif
                            pC++ ;
                            #endif
                        }
                    }

                }
                else
                {

                    //----------------------------------------------------------
                    // Method20: merge A(:,j) and B(:,j), M bitmap/full
                    //----------------------------------------------------------

                    while (pA < pA_end && pB < pB_end)
                    {
                        int64_t iA = Ai [pA] ;
                        int64_t iB = Bi [pB] ;
                        if (iA < iB)
                        {
                            GB_GET_MIJ (iA) ;
                            if (mij)
                            { 
                                #if defined ( GB_PHASE_1_OF_2 )
                                cjnz++ ;
                                #else
                                Ci [pC] = iA ;
                                #ifndef GB_ISO_ADD
                                #ifdef GB_EWISEUNION
                                { 
                                    // C (iA,j) = A(iA,j) + beta
                                    GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                    GB_BINOP (GB_CX (pC), aij, beta_scalar,
                                        iA, j);
                                }
                                #else
                                { 
                                    // C (iA,j) = A (iA,j)
                                    GB_COPY_A_TO_C (GB_CX (pC), Ax, pA, A_iso) ;
                                }
                                #endif
                                #endif
                                pC++ ;
                                #endif
                            }
                            pA++ ;
                        }
                        else if (iA > iB)
                        {
                            GB_GET_MIJ (iB) ;
                            if (mij)
                            { 
                                // C (iB,j) = B (iB,j), or alpha + B(iB,j)
                                #if defined ( GB_PHASE_1_OF_2 )
                                cjnz++ ;
                                #else
                                Ci [pC] = iB ;
                                #ifndef GB_ISO_ADD
                                #ifdef GB_EWISEUNION
                                { 
                                    // C (iB,j) = alpha + B(iB,j)
                                    GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                    GB_BINOP (GB_CX (pC), alpha_scalar, bij,
                                        iB, j) ;
                                }
                                #else
                                { 
                                    // C (iB,j) = B (iB,j)
                                    GB_COPY_B_TO_C (GB_CX (pC), Bx, pB, B_iso) ;
                                }
                                #endif
                                #endif
                                pC++ ;
                                #endif
                            }
                            pB++ ;
                        }
                        else
                        {
                            GB_GET_MIJ (iB) ;
                            if (mij)
                            { 
                                // C (i,j) = A (i,j) + B (i,j)
                                #if defined ( GB_PHASE_1_OF_2 )
                                cjnz++ ;
                                #else
                                Ci [pC] = iB ;
                                #ifndef GB_ISO_ADD
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), aij, bij, iB, j) ;
                                #endif
                                pC++ ;
                                #endif
                            }
                            pA++ ;
                            pB++ ;
                        }
                    }

                    //----------------------------------------------------------
                    // A (:,j) or B (:,j) have entries left; not both
                    //----------------------------------------------------------

                    for ( ; pA < pA_end ; pA++)
                    {
                        int64_t iA = Ai [pA] ;
                        GB_GET_MIJ (iA) ;
                        if (mij)
                        { 
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = iA ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (iA,j) = A(iA,j) + beta
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_BINOP (GB_CX (pC), aij, beta_scalar, iA, j) ;
                            }
                            #else
                            { 
                                // C (iA,j) = A (iA,j)
                                GB_COPY_A_TO_C (GB_CX (pC), Ax, pA, A_iso) ;
                            }
                            #endif
                            #endif
                            pC++ ;
                            #endif
                        }
                    }

                    for ( ; pB < pB_end ; pB++)
                    {
                        int64_t iB = Bi [pB] ;
                        GB_GET_MIJ (iB) ;
                        if (mij)
                        { 
                            // C (iB,j) = B (iB,j), or alpha + B(iB,j)
                            #if defined ( GB_PHASE_1_OF_2 )
                            cjnz++ ;
                            #else
                            Ci [pC] = iB ;
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (iB,j) = alpha + B(iB,j)
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (pC), alpha_scalar, bij,
                                    iB, j) ;
                            }
                            #else
                            { 
                                // C (iB,j) = B (iB,j)
                                GB_COPY_B_TO_C (GB_CX (pC), Bx, pB, B_iso) ;
                            }
                            #endif
                            #endif
                            pC++ ;
                            #endif
                        }
                    }
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

