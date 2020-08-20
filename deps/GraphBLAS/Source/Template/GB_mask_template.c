//------------------------------------------------------------------------------
// GB_mask_template:  phase1 and phase2 for R = masker (M, C, Z)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Computes C<M>=Z or C<!M>=Z, returning the result in R.  The input matrix C
// is not modified.  Effectively, this computes R=C and then R<M>=Z or R<!M>=Z.
// If the C_replace descriptor is enabled, then C has already been cleared, and
// is an empty (but non-NULL) matrix.

// phase1: does not compute R itself, but just counts the # of entries in each
// vector of R.  Fine tasks compute the # of entries in their slice of a
// single vector of R, and the results are cumsum'd.

// phase2: computes R, using the counts computed by phase1.

// FUTURE:: add special cases for C==Z, C==M, and Z==M aliases

//------------------------------------------------------------------------------
// R(i,j) = Z(i,j)
//------------------------------------------------------------------------------

#if defined ( GB_PHASE_1_OF_2 )
    #define GB_COPY_Z                                       \
    {                                                       \
        rjnz++ ;                                            \
    }
#else
    #define GB_COPY_Z                                       \
    {                                                       \
        Ri [pR] = i ;                                       \
        memcpy (Rx +(pR)*rsize, Zx +(pZ)*rsize, rsize) ;    \
        pR++ ;                                              \
    }
#endif

//------------------------------------------------------------------------------
// R(i,j) = C(i,j)
//------------------------------------------------------------------------------

#if defined ( GB_PHASE_1_OF_2 )
    #define GB_COPY_C                                       \
    {                                                       \
        rjnz++ ;                                            \
    }
#else
    #define GB_COPY_C                                       \
    {                                                       \
        Ri [pR] = i ;                                       \
        memcpy (Rx +(pR)*rsize, Cx +(pC)*rsize, rsize) ;    \
        pR++ ;                                              \
    }
#endif

//------------------------------------------------------------------------------
// mask template
//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // get C, Z, M, and R
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Cp = C->p ;
    const int64_t *GB_RESTRICT Ci = C->i ;
    const int64_t vlen = C->vlen ;

    const int64_t *GB_RESTRICT Zp = Z->p ;
    const int64_t *GB_RESTRICT Zi = Z->i ;

    const int64_t *GB_RESTRICT Mp = NULL ;
    // const int64_t *GB_RESTRICT Mh = NULL ;
    const int64_t *GB_RESTRICT Mi = NULL ;
    const GB_void *GB_RESTRICT Mx = NULL ;
    size_t msize = 0 ;
    // int64_t Mnvec = 0 ;
    // bool M_is_hyper = false ;
    if (M != NULL)
    { 
        Mp = M->p ;
        // Mh = M->h ;
        Mi = M->i ;
        Mx = (Mask_struct ? NULL : (M->x)) ;
        msize = M->type->size ;
        // Mnvec = M->nvec ;
        // M_is_hyper = M->is_hyper ;
    }

    #if defined ( GB_PHASE_2_OF_2 )
    const GB_void *GB_RESTRICT Cx = C->x ;
    const GB_void *GB_RESTRICT Zx = Z->x ;
    const int64_t *GB_RESTRICT Rp = R->p ;
    const int64_t *GB_RESTRICT Rh = R->h ;
          int64_t *GB_RESTRICT Ri = R->i ;
          GB_void *GB_RESTRICT Rx = R->x ;
    size_t rsize = R->type->size ;
    #endif

    //--------------------------------------------------------------------------
    // phase1: count entries in each C(:,j); phase2: compute C
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
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
            // get j, the kth vector of R
            //------------------------------------------------------------------

            int64_t j = (Rh == NULL) ? k : Rh [k] ;

            #if defined ( GB_PHASE_1_OF_2 )
            int64_t rjnz = 0 ;
            #else
            int64_t pR, pR_end ;
            if (fine_task)
            { 
                // A fine task computes a slice of R(:,j)
                pR     = TaskList [taskid  ].pC ;
                pR_end = TaskList [taskid+1].pC ;
                ASSERT (Rp [k] <= pR && pR <= pR_end && pR_end <= Rp [k+1]) ;
            }
            else
            { 
                // The vectors of R are never sliced for a coarse task.
                pR     = Rp [k] ;
                pR_end = Rp [k+1] ;
            }
            int64_t rjnz = pR_end - pR ;
            if (rjnz == 0) continue ;
            #endif

            //------------------------------------------------------------------
            // get C(:,j)
            //------------------------------------------------------------------

            int64_t pC = -1, pC_end = -1 ;
            if (fine_task)
            { 
                // A fine task operates on Ci,Cx [pC...pC_end-1], which is
                // a subset of the vector C(:,j)
                pC     = TaskList [taskid].pA ;
                pC_end = TaskList [taskid].pA_end ;
            }
            else
            {
                // A coarse task operates on the entire vector C(:,j)
                int64_t kC = (R_to_C == NULL) ? j : R_to_C [k] ;
                if (kC >= 0)
                { 
                    pC     = Cp [kC] ;
                    pC_end = Cp [kC+1] ;
                }
            }

            int64_t cjnz = pC_end - pC ;        // nnz in C(:,j) for this slice
            bool cdense = (cjnz == len) && (cjnz > 0) ;

            #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DEBUG )
            // get the first index in C(:,j) for this vector
            int64_t iC_first = -1 ;
            if (cjnz > 0) iC_first = Ci [pC] ;
            #endif

            #ifdef GB_DEBUG
            int64_t iC_last = -1 ;
            if (cjnz > 0) iC_last  = Ci [pC_end-1] ;
            #endif

            //------------------------------------------------------------------
            // get Z(:,j)
            //------------------------------------------------------------------

            int64_t pZ = -1, pZ_end = -1 ;
            if (fine_task)
            { 
                // A fine task operates on Zi,Zx [pZ...pZ_end-1], which is
                // a subset of the vector Z(:,j)
                pZ     = TaskList [taskid].pB ;
                pZ_end = TaskList [taskid].pB_end ;
            }
            else
            {
                // A coarse task operates on the entire vector Z(:,j)
                int64_t kZ = (R_to_Z == NULL) ? j : R_to_Z [k] ;
                if (kZ >= 0)
                { 
                    pZ     = Zp [kZ] ;
                    pZ_end = Zp [kZ+1] ;
                }
            }

            int64_t zjnz = pZ_end - pZ ;        // nnz in Z(:,j) for this slice
            bool zdense = (zjnz == len) && (zjnz > 0) ;

            #ifdef GB_DEBUG
            int64_t iZ_first = -1, iZ_last = -1 ;
            if (zjnz > 0)
            {
                iZ_first = Zi [pZ] ;
                iZ_last  = Zi [pZ_end-1] ;
            }
            #endif

            //------------------------------------------------------------------
            // get M(:,j)
            //------------------------------------------------------------------

            int64_t pM = -1, pM_end = -1 ;
            if (fine_task)
            { 
                // A fine task operates on Mi,Mx [pM...pM_end-1], which is
                // a subset of the vector M(:,j)
                pM     = TaskList [taskid].pM ;
                pM_end = TaskList [taskid].pM_end ;
            }
            else
            {
                // A coarse task operates on the entire vector M (:,j)
                int64_t kM = (R_to_M == NULL) ? j : R_to_M [k] ;
                if (kM >= 0)
                { 
                    pM     = Mp [kM] ;
                    pM_end = Mp [kM+1] ;
                }
            }

            int64_t mjnz = pM_end - pM ;    // nnz (M (:,j))
            bool mdense = (mjnz == len) && (mjnz > 0) ;

            // get the first index in M(:,j) for this vector
            int64_t iM_first = -1 ;
            int64_t pM_first = pM ;
            if (mjnz > 0) iM_first = Mi [pM_first] ;

            //------------------------------------------------------------------
            // phase1: count nnz (R(:,j)); phase2: compute R(:,j)
            //------------------------------------------------------------------

            if (mjnz == 0)
            {

                //--------------------------------------------------------------
                // M(:,j) is empty
                //--------------------------------------------------------------

                if (!Mask_comp)
                { 

                    //----------------------------------------------------------
                    // M(:,j) is empty and not complemented
                    //----------------------------------------------------------

                    // R(:,j) = C(:,j), regardless of Z(:,j)
                    #if defined ( GB_PHASE_1_OF_2 )
                    rjnz = cjnz ;
                    #else
                    ASSERT (rjnz == cjnz) ;
                    memcpy (Ri +(pR),       Ci +(pC), cjnz * sizeof (int64_t)) ;
                    memcpy (Rx +(pR)*rsize, Cx +(pC)*rsize, cjnz*rsize) ;
                    #endif

                }
                else
                { 

                    //----------------------------------------------------------
                    // M(:,j) is empty and complemented
                    //----------------------------------------------------------

                    // R(:,j) = Z(:,j), regardless of C(:,j)
                    #if defined ( GB_PHASE_1_OF_2 )
                    rjnz = zjnz ;
                    #else
                    ASSERT (rjnz == zjnz) ;
                    memcpy (Ri +(pR),       Zi +(pZ), zjnz * sizeof (int64_t)) ;
                    memcpy (Rx +(pR)*rsize, Zx +(pZ)*rsize, zjnz*rsize) ;
                    #endif
                }

            }
            else if (cdense && zdense)
            {

                //--------------------------------------------------------------
                // C(:,j) and Z(:,j) dense: thus R(:,j) dense
                //--------------------------------------------------------------

                ASSERT (cjnz == zjnz) ;
                ASSERT (iC_first == iZ_first) ;
                ASSERT (iC_last  == iZ_last ) ;
                #if defined ( GB_PHASE_1_OF_2 )
                rjnz = cjnz ;
                #else
                ASSERT (rjnz == cjnz) ;
                for (int64_t p = 0 ; p < cjnz ; p++)
                {
                    int64_t i = p + iC_first ;
                    Ri [pR + p] = i ;
                    int64_t iM = (pM < pM_end) ? Mi [pM] : INT64_MAX ;
                    bool mij = false ;
                    if (i == iM)
                    { 
                        mij = GB_mcast (Mx, pM, msize) ;
                        pM++ ;
                    }
                    if (Mask_comp) mij = !mij ;
                    if (mij)
                    { 
                        memcpy (Rx +(pR+p)*rsize, Zx +(pZ+p)*rsize, rsize) ;
                    }
                    else
                    { 
                        memcpy (Rx +(pR+p)*rsize, Cx +(pC+p)*rsize, rsize) ;
                    }
                }
                #endif

            }
            else
            {

                //--------------------------------------------------------------
                // 2-way merge of C(:,j) and Z(:,j); binary search of M(:,j)
                //--------------------------------------------------------------

                while (pC < pC_end && pZ < pZ_end)
                {

                    //----------------------------------------------------------
                    // get the next i for R(:,j)
                    //----------------------------------------------------------

                    int64_t iC = Ci [pC] ;
                    int64_t iZ = Zi [pZ] ;
                    int64_t i = GB_IMIN (iC, iZ) ;

                    //----------------------------------------------------------
                    // get M(i,j)
                    //----------------------------------------------------------

                    bool mij = false ;

                    if (mdense)
                    { 

                        //------------------------------------------------------
                        // M(:,j) is dense
                        //------------------------------------------------------

                        // mask is dense, lookup M(i,j)
                        // iM_first == Mi [pM_first]
                        // iM_first + delta == Mi [pM_first + delta]
                        // let i = iM_first + delta
                        // let pM = pM_first + delta
                        // then delta = i - iM_first
                        pM = pM_first + (i - iM_first) ;
                        ASSERT (i == Mi [pM]) ;
                        mij = GB_mcast (Mx, pM, msize) ;
                        // increment pM for the wrapup phase below
                        pM++ ;
                    }
                    else
                    {

                        //------------------------------------------------------
                        // M(:,j) is sparse
                        //------------------------------------------------------

                        // Use GB_SPLIT_BINARY_SEARCH so that pM can be used in
                        // the for loop with index pM in the wrapup phase.
                        int64_t pright = pM_end - 1 ;
                        bool found ;
                        GB_SPLIT_BINARY_SEARCH (i, Mi, pM, pright, found) ;
                        if (found)
                        { 
                            ASSERT (i == Mi [pM]) ;
                            mij = GB_mcast (Mx, pM, msize) ;
                            // increment pM for the wrapup phase below
                            pM++ ;
                        }
                    }

                    if (Mask_comp) mij = !mij ;

                    //----------------------------------------------------------
                    // R(i,j) = C(i,j) or Z(i,j)
                    //----------------------------------------------------------

                    if (iC < iZ)
                    { 
                        // C(i,j) is present but Z(i,j) is not
                        if (!mij) GB_COPY_C ;
                        pC++ ;
                    }
                    else if (iC > iZ)
                    { 
                        // Z(i,j) is present but C(i,j) is not
                        if (mij) GB_COPY_Z ;
                        pZ++ ;
                    }
                    else
                    {
                        // both C(i,j) and Z(i,j) are present
                        if (mij)
                        { 
                            GB_COPY_Z ;
                        }
                        else
                        { 
                            GB_COPY_C ;
                        }
                        pC++ ;
                        pZ++ ;
                    }
                }

                //--------------------------------------------------------------
                // wrapup: C or Z are exhausted, or initially empty
                //--------------------------------------------------------------

                cjnz = pC_end - pC ;    // nnz (C(:,j)) remaining
                zjnz = pZ_end - pZ ;    // nnz (Z(:,j)) remaining
                mjnz = pM_end - pM ;    // nnz (M(:,j)) remaining

                if (cjnz == 0)
                {

                    //----------------------------------------------------------
                    // C(:,j) is empty
                    //----------------------------------------------------------

                    if (!Mask_comp)
                    {

                        //------------------------------------------------------
                        // mask is not complemented
                        //------------------------------------------------------

                        if (mdense)
                        {

                            //--------------------------------------------------
                            // M(:,j) is dense
                            //--------------------------------------------------

                            for ( ; pZ < pZ_end ; pZ++)
                            { 
                                int64_t i = Zi [pZ] ;
                                // mask is dense, lookup M(i,j)
                                pM = pM_first + (i - iM_first) ;
                                ASSERT (i == Mi [pM]) ;
                                bool mij = GB_mcast (Mx, pM, msize) ;
                                if (mij) GB_COPY_Z ;
                            }

                        }
                        else if (zjnz > 32 * mjnz)
                        {

                            //--------------------------------------------------
                            // Z(:,j) is much denser than M(:,j)
                            //--------------------------------------------------

                            // This loop requires pM to start at the first
                            // entry in M(:,j) that has not yet been handled.

                            for ( ; pM < pM_end ; pM++)
                            {
                                if (GB_mcast (Mx, pM, msize))
                                { 
                                    int64_t i = Mi [pM] ;
                                    int64_t pright = pZ_end - 1 ;
                                    bool found ;
                                    GB_BINARY_SEARCH (i, Zi, pZ, pright, found);
                                    if (found) GB_COPY_Z ;
                                }
                            }

                        }
                        else if (mjnz > 32 * zjnz)
                        {

                            //--------------------------------------------------
                            // M(:,j) is much denser than Z(:,j)
                            //--------------------------------------------------

                            for ( ; pZ < pZ_end ; pZ++)
                            { 
                                int64_t i = Zi [pZ] ;
                                bool mij = false ;
                                int64_t pright = pM_end - 1 ;
                                bool found ;
                                GB_BINARY_SEARCH (i, Mi, pM, pright,found) ;
                                if (found) mij = GB_mcast (Mx, pM, msize) ;
                                if (mij) GB_COPY_Z ;
                            }

                        }
                        else
                        {

                            //--------------------------------------------------
                            // M(:,j) and Z(:,j) have about the same # entries
                            //--------------------------------------------------

                            while (pM < pM_end && pZ < pZ_end)
                            {
                                int64_t iM = Mi [pM] ;
                                int64_t i = Zi [pZ] ;
                                if (iM < i)
                                { 
                                    // M(i,j) exists but not Z(i,j)
                                    pM++ ;
                                }
                                else if (i < iM)
                                { 
                                    // Z(i,j) exists but not M(i,j)
                                    pZ++ ;
                                }
                                else
                                { 
                                    // both M(i,j) and Z(i,j) exist
                                    if (GB_mcast (Mx, pM, msize)) GB_COPY_Z ;
                                    pM++ ;
                                    pZ++ ;
                                }
                            }
                        }

                    }
                    else
                    {

                        //------------------------------------------------------
                        // complemented mask, and C(:,j) empty
                        //------------------------------------------------------

                        if (mdense)
                        {

                            //--------------------------------------------------
                            // M(:,j) is dense
                            //--------------------------------------------------

                            for ( ; pZ < pZ_end ; pZ++)
                            { 
                                int64_t i = Zi [pZ] ;
                                // mask is dense, lookup M(i,j)
                                pM = pM_first + (i - iM_first) ;
                                ASSERT (i == Mi [pM]) ;
                                bool mij = GB_mcast (Mx, pM, msize) ;
                                if (!mij) GB_COPY_Z ;   // mask is complemented
                            }
                        }
                        else
                        {

                            //--------------------------------------------------
                            // M(:,j) is sparse
                            //--------------------------------------------------

                            for ( ; pZ < pZ_end ; pZ++)
                            { 
                                int64_t i = Zi [pZ] ;
                                bool mij = false ;
                                int64_t pright = pM_end - 1 ;
                                bool found ;
                                GB_BINARY_SEARCH (i, Mi, pM, pright, found) ;
                                if (found) mij = GB_mcast (Mx, pM, msize) ;
                                if (!mij) GB_COPY_Z ;   // mask is complemented
                            }
                        }
                    }

                }
                else if (zjnz == 0)
                {

                    //----------------------------------------------------------
                    // Z(:,j) is empty
                    //----------------------------------------------------------

                    if (Mask_comp)
                    {

                        //------------------------------------------------------
                        // mask is complemented
                        //------------------------------------------------------

                        if (mdense)
                        {

                            //--------------------------------------------------
                            // M(:,j) is dense
                            //--------------------------------------------------

                            for ( ; pC < pC_end ; pC++)
                            { 
                                int64_t i = Ci [pC] ;
                                // mask is dense, lookup M(i,j)
                                pM = pM_first + (i - iM_first) ;
                                ASSERT (i == Mi [pM]) ;
                                bool mij = GB_mcast (Mx, pM, msize) ;
                                if (mij) GB_COPY_C ;
                            }

                        }
                        else if (cjnz > 32 * mjnz)
                        {

                            //--------------------------------------------------
                            // C(:,j) is much denser than M(:,j)
                            //--------------------------------------------------

                            for ( ; pM < pM_end ; pM++)
                            {
                                if (GB_mcast (Mx, pM, msize))
                                { 
                                    int64_t i = Mi [pM] ;
                                    int64_t pright = pC_end - 1 ;
                                    bool found ;
                                    GB_BINARY_SEARCH (i, Ci, pC, pright, found);
                                    if (found) GB_COPY_C ;
                                }
                            }

                        }
                        else if (mjnz > 32 * cjnz)
                        {

                            //--------------------------------------------------
                            // M(:,j) is much denser than C(:,j)
                            //--------------------------------------------------

                            for ( ; pC < pC_end ; pC++)
                            { 
                                int64_t i = Ci [pC] ;
                                bool mij = false ;
                                int64_t pright = pM_end - 1 ;
                                bool found ;
                                GB_BINARY_SEARCH (i, Mi, pM, pright, found);
                                if (found) mij = GB_mcast (Mx, pM, msize) ;
                                if (mij) GB_COPY_C ;
                            }

                        }
                        else
                        {

                            //--------------------------------------------------
                            // M(:,j) and C(:,j) have about the same # entries
                            //--------------------------------------------------

                            while (pM < pM_end && pC < pC_end)
                            {
                                int64_t iM = Mi [pM] ;
                                int64_t i = Ci [pC] ;
                                if (iM < i)
                                { 
                                    // M(i,j) exists but not C(i,j)
                                    pM++ ;
                                }
                                else if (i < iM)
                                { 
                                    // C(i,j) exists but not M(i,j)
                                    pC++ ;
                                }
                                else
                                { 
                                    // both M(i,j) and C(i,j) exist
                                    if (GB_mcast (Mx, pM, msize)) GB_COPY_C ;
                                    pM++ ;
                                    pC++ ;
                                }
                            }
                        }

                    }
                    else
                    {

                        //------------------------------------------------------
                        // non-complemented mask, and Z(:,j) empty
                        //------------------------------------------------------

                        if (mdense)
                        {

                            //--------------------------------------------------
                            // M(:,j) is dense
                            //--------------------------------------------------

                            for ( ; pC < pC_end ; pC++)
                            { 
                                int64_t i = Ci [pC] ;
                                // mask is dense, lookup M(i,j)
                                pM = pM_first + (i - iM_first) ;
                                ASSERT (i == Mi [pM]) ;
                                bool mij = GB_mcast (Mx, pM, msize) ;
                                if (!mij) GB_COPY_C ;
                            }
                        }
                        else
                        {

                            //--------------------------------------------------
                            // M(:,j) is sparse
                            //--------------------------------------------------

                            for ( ; pC < pC_end ; pC++)
                            { 
                                int64_t i = Ci [pC] ;
                                bool mij = false ;  // M(i,j) false if not present
                                int64_t pright = pM_end - 1 ;
                                bool found ;
                                GB_BINARY_SEARCH (i, Mi, pM, pright, found) ;
                                if (found) mij = GB_mcast (Mx, pM, msize) ;
                                if (!mij) GB_COPY_C ;
                            }
                        }
                    }
                }

                #if defined ( GB_PHASE_2_OF_2 )
                ASSERT (pR == pR_end) ;
                #endif
            }

            //------------------------------------------------------------------
            // final count of nnz (R(:,j))
            //------------------------------------------------------------------

            #if defined ( GB_PHASE_1_OF_2 )
            if (fine_task)
            { 
                TaskList [taskid].pC = rjnz ;
            }
            else
            { 
                Rp [k] = rjnz ;
            }
            #endif
        }
    }
}

