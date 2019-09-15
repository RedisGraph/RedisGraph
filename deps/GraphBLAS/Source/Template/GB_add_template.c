//------------------------------------------------------------------------------
// GB_add_template:  phase1 and phase2 for C=A+B, C<M>=A+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Computes C=A+B (no mask) or C<M>=A+B (mask present and not complemented).
// Does not handle the case C<!M>=A+B.  The complemented mask is handled in
// GB_mask instead.  If present, the mask M is assumed to be very sparse
// compared with A and B.

// phase1: does not compute C itself, but just counts the # of entries in each
// vector of C.  Fine tasks compute the # of entries in their slice of a
// single vector of C, and the results are cumsum'd in GB_task_cumsum.

// phase2: computes C, using the counts computed by phase1.

{

    //--------------------------------------------------------------------------
    // get A, B, M, and C
    //--------------------------------------------------------------------------

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    const int64_t vlen = A->vlen ;

    const int64_t *restrict Bp = B->p ;
    const int64_t *restrict Bi = B->i ;

    const int64_t *restrict Mp = NULL ;
    // const int64_t *restrict Mh = NULL ;
    const int64_t *restrict Mi = NULL ;
    const GB_void *restrict Mx = NULL ;
    GB_cast_function cast_M = NULL ;
    size_t msize = 0 ;
    if (M != NULL)
    { 
        Mp = M->p ;
        // Mh = M->h ;
        Mi = M->i ;
        Mx = M->x ;
        cast_M = GB_cast_factory (GB_BOOL_code, M->type->code) ;
        msize = M->type->size ;
    }

    #if defined ( GB_PHASE_2_OF_2 )
    const GB_ATYPE *restrict Ax = A->x ;
    const GB_ATYPE *restrict Bx = B->x ;
    const int64_t  *restrict Cp = C->p ;
    const int64_t  *restrict Ch = C->h ;
          int64_t  *restrict Ci = C->i ;
          GB_CTYPE *restrict Cx = C->x ;
    #endif

    //--------------------------------------------------------------------------
    // phase1: count entries in each C(:,j); phase2: compute C
    //--------------------------------------------------------------------------

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (int taskid = 0 ; taskid < ntasks ; taskid++)
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

            int64_t j = (Ch == NULL) ? k : Ch [k] ;

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

            // GB_GET_MAPPED_VECTOR (pA, pA_end, pA, pA_end, Ap, j, k, C_to_A) ;
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
                    pA     = Ap [kA] ;
                    pA_end = Ap [kA+1] ;
                }
            }
            // ----

            int64_t ajnz = pA_end - pA ;        // nnz in A(:,j) for this slice
            bool adense = (ajnz == len) ;
            int64_t iA_first = -1, iA_last = -1 ;
            if (ajnz > 0)
            { 
                // get the first and last indices in A(:,j) for this vector
                iA_first = Ai [pA] ;
                iA_last  = Ai [pA_end-1] ;
            }

            //------------------------------------------------------------------
            // get B(:,j)
            //------------------------------------------------------------------

            // GB_GET_MAPPED_VECTOR (pB, pB_end, pB, pB_end, Bp, j, k, C_to_B) ;
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
                    pB     = Bp [kB] ;
                    pB_end = Bp [kB+1] ;
                }
            }
            // ----

            int64_t bjnz = pB_end - pB ;        // nnz in B(:,j) for this slice
            bool bdense = (bjnz == len) ;
            int64_t iB_first = -1, iB_last = -1 ;
            if (bjnz > 0)
            {
                // get the first and last indices in B(:,j) for this vector
                iB_first = Bi [pB] ;
                iB_last  = Bi [pB_end-1] ;
            }

            //------------------------------------------------------------------
            // phase1: count nnz (C (:,j)); phase2: compute C(:,j)
            //------------------------------------------------------------------

            if (M == NULL)
            {

                //--------------------------------------------------------------
                // No mask
                //--------------------------------------------------------------

                // if present, M(:,j) is ignored since !M(:,j) is all true

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
                    // A(:,j) and B(:,j) dense: thus C(:,j) dense
                    //----------------------------------------------------------

                    ASSERT (ajnz == bjnz) ;
                    ASSERT (iA_first == iB_first) ;
                    ASSERT (iA_last  == iB_last ) ;
                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz ;
                    #else
                    ASSERT (cjnz == ajnz) ;
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        Ci [pC + p] = p + iA_first ;
                        GB_GETA (aij, Ax, pA + p) ;
                        GB_GETB (bij, Bx, pB + p) ;
                        GB_BINOP (GB_CX (pC + p), aij, bij) ;
                    }
                    #endif

                }
                else if (adense)
                {

                    //----------------------------------------------------------
                    // A(:,j) dense, B(:,j) sparse: thus C(:,j) dense
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz ;
                    #else
                    ASSERT (cjnz == ajnz) ;
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        Ci [pC + p] = p + iA_first ;
                        GB_COPY_A_TO_C (GB_CX (pC + p), Ax, pA + p) ;
                    }
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        int64_t ii = Bi [pB + p] - iA_first ;
                        GB_GETA (aij, Ax, pA + ii) ;
                        GB_GETB (bij, Bx, pB + p) ;
                        GB_BINOP (GB_CX (pC + ii), aij, bij) ;
                    }
                    #endif

                }
                else if (bdense)
                {

                    //----------------------------------------------------------
                    // A(:,j) sparse, B(:,j) dense: thus C(:,j) dense
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = bjnz ;
                    #else
                    ASSERT (cjnz == bjnz) ;
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        Ci [pC + p] = p + iB_first ;
                        GB_COPY_B_TO_C (GB_CX (pC + p), Bx, pB + p) ;
                    }
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        int64_t ii = Ai [pA + p] - iB_first ;
                        GB_GETA (aij, Ax, pA + p) ;
                        GB_GETB (bij, Bx, pB + ii) ;
                        GB_BINOP (GB_CX (pC + ii), aij, bij) ;
                    }
                    #endif

                }
                else if (ajnz == 0)
                {

                    //----------------------------------------------------------
                    // A(:,j) is empty
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = bjnz ;
                    #else
                    ASSERT (cjnz == bjnz) ;
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        Ci [pC + p] = Bi [pB + p] ;
                        GB_COPY_B_TO_C (GB_CX (pC + p), Bx, pB + p) ;
                    }
                    #endif

                }
                else if (bjnz == 0)
                {

                    //----------------------------------------------------------
                    // B(:,j) is empty
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz ;
                    #else
                    ASSERT (cjnz == ajnz) ;
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        Ci [pC + p] = Ai [pA + p] ;
                        GB_COPY_A_TO_C (GB_CX (pC + p), Ax, pA + p) ;
                    }
                    #endif

                }
                else if (iA_last < iB_first)
                {

                    //----------------------------------------------------------
                    // last entry of A(:,j) comes before first entry of B(:,j)
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz + bjnz ;
                    #else
                    ASSERT (cjnz == ajnz + bjnz) ;
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        Ci [pC + p] = Ai [pA + p] ;
                        GB_COPY_A_TO_C (GB_CX (pC + p), Ax, pA + p) ;
                    }
                    pC += ajnz ;
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        Ci [pC + p] = Bi [pB + p] ;
                        GB_COPY_B_TO_C (GB_CX (pC + p), Bx, pB + p) ;
                    }
                    #endif

                }
                else if (iB_last < iA_first)
                {

                    //----------------------------------------------------------
                    // last entry of B(:,j) comes before first entry of A(:,j)
                    //----------------------------------------------------------

                    #if defined ( GB_PHASE_1_OF_2 )
                    cjnz = ajnz + bjnz ;
                    #else
                    ASSERT (cjnz == ajnz + bjnz) ;
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        Ci [pC + p] = Bi [pB + p] ;
                        GB_COPY_B_TO_C (GB_CX (pC + p), Bx, pB + p) ;
                    }
                    pC += bjnz ;
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        Ci [pC + p] = Ai [pA + p] ;
                        GB_COPY_A_TO_C (GB_CX (pC + p), Ax, pA + p) ;
                    }
                    #endif

                }

                #if defined ( GB_PHASE_1_OF_2 )
                else if (ajnz > 32 * bjnz)
                {

                    //----------------------------------------------------------
                    // A(:,j) is much denser than B(:,j)
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
                    // B(:,j) is much denser than A(:,j)
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
                    // A(:,j) and B(:,j) have about the same # of entries
                    //----------------------------------------------------------

                    while (pA < pA_end && pB < pB_end)
                    {
                        int64_t iA = Ai [pA] ;
                        int64_t iB = Bi [pB] ;
                        if (iA < iB)
                        { 
                            // C (iA,j) = A (iA,j)
                            #if defined ( GB_PHASE_2_OF_2 )
                            Ci [pC] = iA ;
                            GB_COPY_A_TO_C (GB_CX (pC), Ax, pA) ;
                            #endif
                            pA++ ;
                        }
                        else if (iA > iB)
                        { 
                            // C (iB,j) = B (iB,j)
                            #if defined ( GB_PHASE_2_OF_2 )
                            Ci [pC] = iB ;
                            GB_COPY_B_TO_C (GB_CX (pC), Bx, pB) ;
                            #endif
                            pB++ ;
                        }
                        else
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            #if defined ( GB_PHASE_2_OF_2 )
                            Ci [pC] = iB ;
                            GB_GETA (aij, Ax, pA) ;
                            GB_GETB (bij, Bx, pB) ;
                            GB_BINOP (GB_CX (pC), aij, bij) ;
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
                    for (int64_t p = 0 ; p < ajnz ; p++)
                    { 
                        // C (i,j) = A (i,j)
                        Ci [pC + p] = Ai [pA + p] ;
                        GB_COPY_A_TO_C (GB_CX (pC + p), Ax, pA + p) ;
                    }
                    for (int64_t p = 0 ; p < bjnz ; p++)
                    { 
                        // C (i,j) = B (i,j)
                        Ci [pC + p] = Bi [pB + p] ;
                        GB_COPY_B_TO_C (GB_CX (pC + p), Bx, pB + p) ;
                    }
                    ASSERT (pC + ajnz + bjnz == pC_end) ;
                    #endif
                }

            }
            else
            {

                //--------------------------------------------------------------
                // Mask is present
                //--------------------------------------------------------------

                int64_t pM = -1 ;
                int64_t pM_end = -1 ;
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
                    if (Ch_is_Mh)
                    { 
                        // Ch is the same as Mh (a deep copy)
                        ASSERT (Ch != NULL) ;
                        ASSERT (M->h != NULL) ;
                        ASSERT (Ch [k] == M->h [k]) ;
                        kM = k ;
                    }
                    else
                    { 
                        kM = (C_to_M == NULL) ? j : C_to_M [k] ;
                    }
                    if (kM >= 0)
                    { 
                        pM     = Mp [kM] ;
                        pM_end = Mp [kM+1] ;
                    }
                }

                //--------------------------------------------------------------
                // C(:,j)<M(:,j)> = A(:,j) + B (:,j)
                //--------------------------------------------------------------

                for ( ; pM < pM_end ; pM++)
                {

                    //----------------------------------------------------------
                    // get M(i,j) for A(i,j) + B (i,j)
                    //----------------------------------------------------------

                    int64_t i = Mi [pM] ;
                    bool mij ;
                    cast_M (&mij, Mx +(pM*msize), 0) ;
                    if (!mij) continue ;

                    //----------------------------------------------------------
                    // get A(i,j)
                    //----------------------------------------------------------

                    int64_t apright = pA_end - 1 ;
                    bool afound ;
                    GB_BINARY_SEARCH (i, Ai, pA, apright, afound) ;

                    //----------------------------------------------------------
                    // get B(i,j)
                    //----------------------------------------------------------

                    int64_t bpright = pB_end - 1 ;
                    bool bfound ;
                    GB_BINARY_SEARCH (i, Bi, pB, bpright, bfound) ;

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
                        GB_GETA (aij, Ax, pA) ;
                        GB_GETB (bij, Bx, pB) ;
                        GB_BINOP (GB_CX (pC), aij, bij) ;
                        pC++ ;
                        #endif
                    }
                    else if (afound)
                    { 
                        // C (i,j) = A (i,j)
                        #if defined ( GB_PHASE_1_OF_2 )
                        cjnz++ ;
                        #else
                        Ci [pC] = i ;
                        GB_COPY_A_TO_C (GB_CX (pC), Ax, pA) ;
                        pC++ ;
                        #endif
                    }
                    else if (bfound)
                    { 
                        // C (i,j) = B (i,j)
                        #if defined ( GB_PHASE_1_OF_2 )
                        cjnz++ ;
                        #else
                        Ci [pC] = i ;
                        GB_COPY_B_TO_C (GB_CX (pC), Bx, pB) ;
                        pC++ ;
                        #endif
                    }
                }

                #if defined ( GB_PHASE_2_OF_2 )
                ASSERT (pC == pC_end) ;
                #endif

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

