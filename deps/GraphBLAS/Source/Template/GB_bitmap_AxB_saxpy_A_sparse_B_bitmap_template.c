//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy_A_sparse_B_bitmap: C<#M>+=A*B, C bitmap, M any format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    if (use_coarse_tasks)
    {

        //----------------------------------------------------------------------
        // C<#M> += A*B using coarse tasks
        //----------------------------------------------------------------------

        // number of columns in the workspace for each task
        #define GB_PANEL_SIZE 4

        //----------------------------------------------------------------------
        // allocate workspace for each task
        //----------------------------------------------------------------------

        GH_slice = GB_MALLOC (2*ntasks, int64_t) ;
        if (GH_slice == NULL)
        {
            // out of memory
            GB_FREE_WORK ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        int64_t *GB_RESTRICT G_slice = GH_slice ;
        int64_t *GB_RESTRICT H_slice = GH_slice + ntasks ;

        int64_t gwork = 0 ;
        int64_t hwork = 0 ;
        int tid ;
        for (tid = 0 ; tid < ntasks ; tid++)
        {
            int64_t jstart, jend ;
            GB_PARTITION (jstart, jend, bvdim, tid, ntasks) ;
            int64_t jtask = jend - jstart ;
            int64_t jpanel = GB_IMIN (jtask, GB_PANEL_SIZE) ;
            G_slice [tid] = gwork ;
            H_slice [tid] = hwork ;
            if (jpanel > 1)
            {
                // no need to allocate workspace for Gb and Gx if jpanel == 1
                gwork += jpanel ;
            }
            hwork += jpanel ;
        }

        int64_t bvlenx = (B_is_pattern ? 0 : bvlen) * GB_BSIZE ;
        int64_t cvlenx = (GB_IS_ANY_PAIR_SEMIRING ? 0 : cvlen) * GB_CSIZE ;
        int64_t bvlenb = (GB_B_IS_BITMAP ? bvlen : 0) ;
        size_t gfspace = gwork * bvlenb ;
        size_t wfspace = gfspace + hwork * cvlen ;
        size_t wbxspace = gwork * bvlenx ;
        size_t wcxspace = hwork * cvlenx ;
        Wf = GB_MALLOC (wfspace, int8_t) ;
        Wbx = GB_MALLOC (wbxspace, GB_void) ;
        Wcx = GB_MALLOC (wcxspace, GB_void) ;
        if (Wf == NULL || Wcx == NULL || Wbx == NULL)
        {
            // out of memory
            GB_FREE_WORK ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // C<#M> += A*B
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:cnvals)
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            //------------------------------------------------------------------
            // determine the vectors of B and C for this coarse task
            //------------------------------------------------------------------

            int64_t jstart, jend ;
            GB_PARTITION (jstart, jend, bvdim, tid, ntasks) ;
            int64_t jtask = jend - jstart ;
            int64_t jpanel = GB_IMIN (jtask, GB_PANEL_SIZE) ;
            int64_t task_cnvals = 0 ;

            //------------------------------------------------------------------
            // get the workspace for this task
            //------------------------------------------------------------------

            // Gb and Gx workspace to load the panel of B
            int8_t   *GB_RESTRICT Gb = Wf  + G_slice [tid] * bvlenb ;
            GB_BTYPE *GB_RESTRICT Gx = (GB_BTYPE *)
                (Wbx + G_slice [tid] * bvlenx) ;

            // Hf and Hx workspace to compute the panel of C
            int8_t   *GB_RESTRICT Hf = Wf  + (H_slice [tid] * cvlen) + gfspace ;
            GB_CTYPE *GB_RESTRICT Hx = (GB_CTYPE *)
                (Wcx +  H_slice [tid] * cvlenx) ;
            #if GB_IS_PLUS_FC32_MONOID
            float  *GB_RESTRICT Hx_real = (float *) Hx ;
            float  *GB_RESTRICT Hx_imag = Hx_real + 1 ;
            #elif GB_IS_PLUS_FC64_MONOID
            double *GB_RESTRICT Hx_real = (double *) Hx ;
            double *GB_RESTRICT Hx_imag = Hx_real + 1 ;
            #endif

            //------------------------------------------------------------------
            // clear the panel
            //------------------------------------------------------------------

            memset (Hf, 0, jpanel * cvlen) ;

            //------------------------------------------------------------------
            // C<#M>(:,jstart:jend-1) += A * B(:,jstart:jend-1) by panel
            //------------------------------------------------------------------

            for (int64_t j1 = jstart ; j1 < jend ; j1 += jpanel)
            {

                //--------------------------------------------------------------
                // get the panel of np vectors j1:j2-1
                //--------------------------------------------------------------

                int64_t j2 = GB_IMIN (jend, j1 + jpanel) ;
                int64_t np = j2 - j1 ;

                //--------------------------------------------------------------
                // load and transpose B(:,j1:j2-1) for one panel
                //--------------------------------------------------------------

                #if GB_B_IS_BITMAP
                {
                    if (np == 1)
                    {
                        // no need to load a single vector of B
                        Gb = (int8_t *) (Bb + (j1 * bvlen)) ;
                    }
                    else
                    {
                        // load and transpose the bitmap of B(:,j1:j2-1)
                        for (int64_t jj = 0 ; jj < np ; jj++)
                        {
                            int64_t j = j1 + jj ;
                            for (int64_t i = 0 ; i < bvlen ; i++)
                            { 
                                Gb [i*np + jj] = Bb [i + j * bvlen] ;
                            }
                        }
                    }
                }
                #endif

                if (!B_is_pattern)
                {
                    if (np == 1)
                    {
                        // no need to load a single vector of B
                        GB_void *GB_RESTRICT Bx = B->x ;
                        Gx = (GB_BTYPE *) (Bx + (j1 * bvlen) * GB_BSIZE) ;
                    }
                    else
                    {
                        // load and transpose the values of B(:,j1:j2-1)
                        for (int64_t jj = 0 ; jj < np ; jj++)
                        {
                            int64_t j = j1 + jj ;
                            for (int64_t i = 0 ; i < bvlen ; i++)
                            { 
                                // G(i,jj) = B(i,j), and change storage order
                                int64_t pG = i*np + jj ;
                                int64_t pB = i + j * bvlen ;
                                GB_LOADB (Gx, pG, Bx, pB) ;
                            }
                        }
                    }
                }

                //--------------------------------------------------------------
                // H = A*G for one panel
                //--------------------------------------------------------------

                for (int64_t kA = 0 ; kA < anvec ; kA++)
                {

                    //----------------------------------------------------------
                    // get A(:,k)
                    //----------------------------------------------------------

                    int64_t k = GBH (Ah, kA) ;
                    int64_t pA = Ap [kA] ;
                    int64_t pA_end = Ap [kA+1] ;
                    int64_t pG = k * np ;

                    #undef  GB_MULT_A_ik_G_kjj
                    #if GB_IS_PAIR_MULTIPLIER
                        // t = A(i,k) * G (k,jj) is always equal to 1
                        #define GB_MULT_A_ik_G_kjj(jj)
                    #else
                        // t = A(i,k) * G (k,jj)
                        GB_CIJ_DECLARE (t) ;
                        #define GB_MULT_A_ik_G_kjj(jj)                      \
                            GB_GETB (gkj, Gx, pG+jj) ;                      \
                            GB_MULT (t, aik, gkj, i, k, j1 + jj) ;
                    #endif

                    #undef  GB_HX_COMPUTE
                    #define GB_HX_COMPUTE(jj)                               \
                    {                                                       \
                        /* H (i,jj) += A(i,k)*G(k,jj) */                    \
                        if (!GB_B_IS_BITMAP || Gb [pG+jj])                  \
                        {                                                   \
                            GB_MULT_A_ik_G_kjj (jj) ;                       \
                            if (Hf [pH+jj] == 0)                            \
                            {                                               \
                                /* H(i,jj) is a new entry */                \
                                GB_HX_WRITE (pH+jj, t) ; /* Hx(i,jj)=t */   \
                                Hf [pH+jj] = 1 ;                            \
                            }                                               \
                            else                                            \
                            {                                               \
                                /* H(i,jj) is already present */            \
                                GB_HX_UPDATE (pH+jj, t) ; /* Hx(i,jj)+=t */ \
                            }                                               \
                        }                                                   \
                    }

                    #undef  GB_LOAD_A_ij
                    #define GB_LOAD_A_ij                                    \
                        int64_t i = Ai [pA] ;                               \
                        GB_GETA (aik, Ax, pA) ;                             \
                        int64_t pH = i * np ;

                    //----------------------------------------------------------
                    // H += A(:,k)*G(k,:)
                    //----------------------------------------------------------

                    #if GB_B_IS_BITMAP
                    bool gb = false ;
                    switch (np)
                    {
                        case 4 : gb  = Gb [pG+3] ;
                        case 3 : gb |= Gb [pG+2] ;
                        case 2 : gb |= Gb [pG+1] ;
                        case 1 : gb |= Gb [pG  ] ; 
                        default: ;
                    }
                    if (gb)
                    #endif
                    {
                        switch (np)
                        {

                            case 4 : 
                                for ( ; pA < pA_end ; pA++)
                                {
                                    GB_LOAD_A_ij ;
                                    GB_HX_COMPUTE (0) ;
                                    GB_HX_COMPUTE (1) ;
                                    GB_HX_COMPUTE (2) ;
                                    GB_HX_COMPUTE (3) ;
                                }
                                break ;

                            case 3 : 
                                for ( ; pA < pA_end ; pA++)
                                {
                                    GB_LOAD_A_ij ;
                                    GB_HX_COMPUTE (0) ;
                                    GB_HX_COMPUTE (1) ;
                                    GB_HX_COMPUTE (2) ;
                                }
                                break ;

                            case 2 : 
                                for ( ; pA < pA_end ; pA++)
                                {
                                    GB_LOAD_A_ij ;
                                    GB_HX_COMPUTE (0) ;
                                    GB_HX_COMPUTE (1) ;
                                }
                                break ;

                            case 1 : 
                                for ( ; pA < pA_end ; pA++)
                                {
                                    GB_LOAD_A_ij ;
                                    GB_HX_COMPUTE (0) ;
                                }
                                break ;
                            default:;
                        }
                    }

                    #undef  GB_MULT_A_ik_G_kjj
                    #undef  GB_HX_COMPUTE
                    #undef  GB_LOAD_A_ij
                }

                //--------------------------------------------------------------
                // C<#M>(:,j1:j2-1) += H
                //--------------------------------------------------------------

                for (int64_t jj = 0 ; jj < np ; jj++)
                {

                    //----------------------------------------------------------
                    // C<#M>(:,j) += H (:,jj)
                    //----------------------------------------------------------

                    int64_t j = j1 + jj ;
                    int64_t pC_start = j * avlen ;  // get pointer to C(:,j)

                    for (int64_t i = 0 ; i < cvlen ; i++)
                    {
                        int64_t pC = pC_start + i ;     // pointer to C(i,j)
                        int64_t pH = i * np + jj ;      // pointer to H(i,jj)
                        if (!Hf [pH]) continue ;
                        Hf [pH] = 0 ;                   // clear the panel
                        int8_t cb = Cb [pC] ;

                        //------------------------------------------------------
                        // check M(i,j)
                        //------------------------------------------------------

                        #if GB_MASK_IS_SPARSE_OR_HYPER

                            // M is sparse or hypersparse
                            bool mij = ((cb & 2) != 0) ^ Mask_comp ;
                            if (!mij) continue ;
                            cb = (cb & 1) ;

                        #elif GB_MASK_IS_BITMAP_OR_FULL

                            // M is bitmap or full
                            GB_GET_M_ij (pC) ;
                            mij = mij ^ Mask_comp ;
                            if (!mij) continue ;

                        #endif

                        //------------------------------------------------------
                        // C(i,j) += H(i,jj)
                        //------------------------------------------------------

                        if (cb == 0)
                        { 
                            // C(i,j) = H(i,jj)
                            #if GB_IS_ANY_PAIR_SEMIRING
                            Cx [pC] = GB_CTYPE_CAST (1, 0) ;    // C(i,j) = 1
                            #else
                            GB_CIJ_GATHER (pC, pH) ;
                            #endif
                            Cb [pC] = keep ;
                            task_cnvals++ ;
                        }
                        else
                        {
                            // Currently, the matrix C is a newly allocated
                            // matrix, not the C_in input matrix to GrB_mxm.
                            // As a result, this condition is not used.  It
                            // will be in the future when this method is
                            // modified to modify C in-place.
                            ASSERT (GB_DEAD_CODE) ;
                            // C(i,j) += H(i,jj)
                            GB_CIJ_GATHER_UPDATE (pC, pH) ;
                        }
                    }
                }
            }
            cnvals += task_cnvals ;
        }

        #undef GB_PANEL_SIZE

    }
    else if (use_atomics)
    {

        //----------------------------------------------------------------------
        // C<#M> += A*B using fine tasks and atomics
        //----------------------------------------------------------------------

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:cnvals)
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            //------------------------------------------------------------------
            // determine the vector of B and C for this fine task
            //------------------------------------------------------------------

            // The fine task operates on C(:,j) and B(:,j).  Its fine task
            // id ranges from 0 to nfine_tasks_per_vector-1, and determines
            // which slice of A to operate on.

            int64_t j    = tid / nfine_tasks_per_vector ;
            int fine_tid = tid % nfine_tasks_per_vector ;
            int64_t kfirst = A_slice [fine_tid] ;
            int64_t klast = A_slice [fine_tid + 1] ;
            int64_t pB_start = j * bvlen ;      // pointer to B(:,j)
            int64_t pC_start = j * avlen ;      // pointer to C(:,j)
            GB_GET_T_FOR_SECONDJ ;              // t = j or j+1 for SECONDJ*
            int64_t task_cnvals = 0 ;

            // for Hx Gustavason workspace: use C(:,j) in-place:
            GB_CTYPE *GB_RESTRICT Hx = (GB_CTYPE *)
                (((GB_void *) Cx) + (pC_start * GB_CSIZE)) ;
            #if GB_IS_PLUS_FC32_MONOID || GB_IS_ANY_FC32_MONOID
            float  *GB_RESTRICT Hx_real = (float *) Hx ;
            float  *GB_RESTRICT Hx_imag = Hx_real + 1 ;
            #elif GB_IS_PLUS_FC64_MONOID || GB_IS_ANY_FC64_MONOID
            double *GB_RESTRICT Hx_real = (double *) Hx ;
            double *GB_RESTRICT Hx_imag = Hx_real + 1 ;
            #endif

            //------------------------------------------------------------------
            // C<#M>(:,j) += A(:,k1:k2) * B(k1:k2,j)
            //------------------------------------------------------------------

            for (int64_t kk = kfirst ; kk < klast ; kk++)
            {

                //--------------------------------------------------------------
                // C<#M>(:,j) += A(:,k) * B(k,j)
                //--------------------------------------------------------------

                int64_t k = GBH (Ah, kk) ;      // k in range k1:k2
                int64_t pB = pB_start + k ;     // get pointer to B(k,j)
                if (!GBB (Bb, pB)) continue ;   
                int64_t pA = Ap [kk] ;
                int64_t pA_end = Ap [kk+1] ;
                GB_GET_B_kj ;                   // bkj = B(k,j)

                for ( ; pA < pA_end ; pA++)
                {

                    //----------------------------------------------------------
                    // get A(i,k) and C(i,j)
                    //----------------------------------------------------------

                    int64_t i = Ai [pA] ;       // get A(i,k) index
                    int64_t pC = pC_start + i ; // get C(i,j) pointer
                    int8_t cb ;

                    //----------------------------------------------------------
                    // C<#M>(i,j) += A(i,k) * B(k,j)
                    //----------------------------------------------------------

                    #if GB_MASK_IS_SPARSE_OR_HYPER
                    { 

                        //------------------------------------------------------
                        // M is sparse, and scattered into the C bitmap
                        //------------------------------------------------------

                        // finite-state machine in Cb [pC]:
                        // 0:   cij not present, mij zero
                        // 1:   cij present, mij zero (keep==1 for !M)
                        // 2:   cij not present, mij one
                        // 3:   cij present, mij one (keep==3 for M)
                        // 7:   cij is locked

                        #if GB_HAS_ATOMIC
                        { 
                            // if C(i,j) is already present and can be modified
                            // (cb==keep), and the monoid can be done
                            // atomically, then do the atomic update.  No need
                            // to modify Cb [pC].
                            GB_ATOMIC_READ
                            cb = Cb [pC] ;          // grab the entry
                            if (cb == keep)
                            { 
                                #if !GB_IS_ANY_MONOID
                                GB_MULT_A_ik_B_kj ;     // t = A(i,k) * B(k,j)
                                GB_ATOMIC_UPDATE_HX (i, t) ;    // C(i,j) += t
                                #endif
                                continue ;          // C(i,j) has been updated
                            }
                        }
                        #endif

                        do  // lock the entry
                        { 
                            // do this atomically:
                            // { cb = Cb [pC] ;  Cb [pC] = 7 ; }
                            GB_ATOMIC_CAPTURE_INT8 (cb, Cb [pC], 7) ;
                        } while (cb == 7) ; // lock owner gets 0, 1, 2, or 3
                        if (cb == keep-1)
                        { 
                            // C(i,j) is a new entry
                            GB_MULT_A_ik_B_kj ;             // t = A(i,k)*B(k,j)
                            #if GB_IS_ANY_PAIR_SEMIRING
                            GB_ATOMIC_SET_HX_ONE (i) ;      // C(i,j) = 1
                            #else
                            GB_ATOMIC_WRITE_HX (i, t) ;     // C(i,j) = t
                            #endif
                            task_cnvals++ ;
                            cb = keep ;                     // keep the entry
                        }
                        else if (cb == keep)
                        { 
                            // C(i,j) is already present
                            #if !GB_IS_ANY_MONOID
                            GB_MULT_A_ik_B_kj ;             // t = A(i,k)*B(k,j)
                            GB_ATOMIC_UPDATE_HX (i, t) ;    // C(i,j) += t
                            #endif
                        }
                        GB_ATOMIC_WRITE
                        Cb [pC] = cb ;                  // unlock the entry

                    }
                    #else
                    { 

                        //------------------------------------------------------
                        // M is not present, or bitmap/full
                        //------------------------------------------------------

                        // finite-state machine in Cb [pC]:
                        // 0:   cij not present; can be written
                        // 1:   cij present; can be updated
                        // 7:   cij is locked

                        #if GB_MASK_IS_BITMAP_OR_FULL
                        { 
                            // M is bitmap or full, and not in C bitmap.
                            // Do not modify C(i,j) if not permitted by the mask
                            GB_GET_M_ij (pC) ;
                            mij = mij ^ Mask_comp ;
                            if (!mij) continue ;
                        }
                        #endif

                        //------------------------------------------------------
                        // C(i,j) += A(i,j) * B(k,j)
                        //------------------------------------------------------

                        #if GB_HAS_ATOMIC
                        { 
                            // if C(i,j) is already present (cb==1), and the
                            // monoid can be done atomically, then do the
                            // atomic update.  No need to modify Cb [pC].
                            GB_ATOMIC_READ
                            cb = Cb [pC] ;          // grab the entry
                            if (cb == 1)
                            { 
                                #if !GB_IS_ANY_MONOID
                                GB_MULT_A_ik_B_kj ;     // t = A(i,k) * B(k,j)
                                GB_ATOMIC_UPDATE_HX (i, t) ;    // C(i,j) += t
                                #endif
                                continue ;          // C(i,j) has been updated
                            }
                        }
                        #endif

                        do  // lock the entry
                        { 
                            // do this atomically:
                            // { cb = Cb [pC] ;  Cb [pC] = 7 ; }
                            GB_ATOMIC_CAPTURE_INT8 (cb, Cb [pC], 7) ;
                        } while (cb == 7) ; // lock owner gets 0 or 1
                        if (cb == 0)
                        { 
                            // C(i,j) is a new entry
                            GB_MULT_A_ik_B_kj ;             // t = A(i,k)*B(k,j)
                            #if GB_IS_ANY_PAIR_SEMIRING
                            GB_ATOMIC_SET_HX_ONE (i) ;      // C(i,j) = 1
                            #else
                            GB_ATOMIC_WRITE_HX (i, t) ;     // C(i,j) = t
                            #endif
                            task_cnvals++ ;
                        }
                        else // cb == 1
                        { 
                            // C(i,j) is already present
                            #if !GB_IS_ANY_MONOID
                            GB_MULT_A_ik_B_kj ;             // t = A(i,k)*B(k,j)
                            GB_ATOMIC_UPDATE_HX (i, t) ;    // C(i,j) += t
                            #endif
                        }
                        GB_ATOMIC_WRITE
                        Cb [pC] = 1 ;               // unlock the entry

                    }
                    #endif

                }
            }
            cnvals += task_cnvals ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C<#M> += A*B using fine tasks and workspace, with no atomics
        //----------------------------------------------------------------------

        // Each fine task is given size-cvlen workspace to compute its result
        // in the first phase, W(:,tid) = A(:,k1:k2) * B(k1:k2,j), where k1:k2
        // is defined by the fine_tid of the task.  The workspaces are then
        // summed into C in the second phase.

        //----------------------------------------------------------------------
        // allocate workspace
        //----------------------------------------------------------------------

        size_t workspace = cvlen * ntasks ;
        Wf = GB_CALLOC (workspace, int8_t) ;
        size_t cxsize = (GB_IS_ANY_PAIR_SEMIRING) ? 0 : GB_CSIZE ;
        Wcx = GB_MALLOC (workspace * cxsize, GB_void) ;
        if (Wf == NULL || Wcx == NULL)
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // first phase: W (:,tid) = A (:,k1:k2) * B (k2:k2,j) for each fine task
        //----------------------------------------------------------------------

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            //------------------------------------------------------------------
            // determine the vector of B and C for this fine task
            //------------------------------------------------------------------

            // The fine task operates on C(:,j) and B(:,j).  Its fine task
            // id ranges from 0 to nfine_tasks_per_vector-1, and determines
            // which slice of A to operate on.

            int64_t j    = tid / nfine_tasks_per_vector ;
            int fine_tid = tid % nfine_tasks_per_vector ;
            int64_t kfirst = A_slice [fine_tid] ;
            int64_t klast = A_slice [fine_tid + 1] ;
            int64_t pB_start = j * bvlen ;      // pointer to B(:,j)
            int64_t pC_start = j * avlen ;      // pointer to C(:,j), for bitmap
            int64_t pW_start = tid * avlen ;    // pointer to W(:,tid)
            GB_GET_T_FOR_SECONDJ ;              // t = j or j+1 for SECONDJ*
            int64_t task_cnvals = 0 ;

            // for Hf and Hx Gustavason workspace: use W(:,tid):
            int8_t   *GB_RESTRICT Hf = Wf + pW_start ;
            GB_CTYPE *GB_RESTRICT Hx = (GB_CTYPE *) 
                (Wcx + (pW_start * cxsize)) ;
            #if GB_IS_PLUS_FC32_MONOID
            float  *GB_RESTRICT Hx_real = (float *) Hx ;
            float  *GB_RESTRICT Hx_imag = Hx_real + 1 ;
            #elif GB_IS_PLUS_FC64_MONOID
            double *GB_RESTRICT Hx_real = (double *) Hx ;
            double *GB_RESTRICT Hx_imag = Hx_real + 1 ;
            #endif

            //------------------------------------------------------------------
            // W<#M> = A(:,k1:k2) * B(k1:k2,j)
            //------------------------------------------------------------------

            for (int64_t kk = kfirst ; kk < klast ; kk++)
            {

                //--------------------------------------------------------------
                // W<#M>(:,tid) += A(:,k) * B(k,j)
                //--------------------------------------------------------------

                int64_t k = GBH (Ah, kk) ;      // k in range k1:k2
                int64_t pB = pB_start + k ;     // get pointer to B(k,j)
                if (!GBB (Bb, pB)) continue ;   
                int64_t pA = Ap [kk] ;
                int64_t pA_end = Ap [kk+1] ;
                GB_GET_B_kj ;                   // bkj = B(k,j)

                for ( ; pA < pA_end ; pA++)
                {

                    //----------------------------------------------------------
                    // get A(i,k)
                    //----------------------------------------------------------

                    int64_t i = Ai [pA] ;       // get A(i,k) index

                    //----------------------------------------------------------
                    // check M(i,j)
                    //----------------------------------------------------------

                    #if GB_MASK_IS_SPARSE_OR_HYPER
                    { 
                        // M is sparse or hypersparse
                        int64_t pC = pC_start + i ;
                        int8_t cb = Cb [pC] ;
                        bool mij = ((cb & 2) != 0) ^ Mask_comp ;
                        if (!mij) continue ;
                    }
                    #elif GB_MASK_IS_BITMAP_OR_FULL
                    { 
                        // M is bitmap or full
                        int64_t pC = pC_start + i ;
                        GB_GET_M_ij (pC) ;
                        mij = mij ^ Mask_comp ;
                        if (!mij) continue ;
                    }
                    #endif

                    //----------------------------------------------------------
                    // W<#M>(i) += A(i,k) * B(k,j)
                    //----------------------------------------------------------

                    #if GB_IS_ANY_PAIR_SEMIRING
                    { 
                        // Hx is not used; Cx [...] = 1 is done below
                        Hf [i] = 1 ;
                    }
                    #else
                    {
                        GB_MULT_A_ik_B_kj ;         // t = A(i,k)*B(k,j)
                        if (Hf [i] == 0)
                        { 
                            // W(i,j) is a new entry
                            GB_HX_WRITE (i, t) ;    // Hx(i) = t
                            Hf [i] = 1 ;
                        }
                        else
                        { 
                            // W(i) is already present
                            GB_HX_UPDATE (i, t) ;   // Hx(i) += t
                        }
                    }
                    #endif
                }
            }
        }

        //----------------------------------------------------------------------
        // second phase: C<#M> += reduce (W)
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:cnvals)
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            //------------------------------------------------------------------
            // determine the W and C for this fine task
            //------------------------------------------------------------------

            // The fine task operates on C(i1:i2,j) and W(i1:i2,w1:w2), where
            // i1:i2 is defined by the fine task id.  Its fine task id ranges
            // from 0 to nfine_tasks_per_vector-1.
            
            // w1:w2 are the updates to C(:,j), where w1:w2 =
            // [j*nfine_tasks_per_vector : (j+1)*nfine_tasks_per_vector-1].

            int64_t j    = tid / nfine_tasks_per_vector ;
            int fine_tid = tid % nfine_tasks_per_vector ;
            int64_t istart, iend ;
            GB_PARTITION (istart, iend, cvlen, fine_tid,
                nfine_tasks_per_vector) ;
            int64_t pC_start = j * cvlen ;          // pointer to C(:,j)
            int64_t wstart = j * nfine_tasks_per_vector ;
            int64_t wend = (j + 1) * nfine_tasks_per_vector ;
            int64_t task_cnvals = 0 ;

            // Hx = (typecasted) Wcx workspace, use Wf as-is
            GB_CTYPE *GB_RESTRICT Hx = ((GB_CTYPE *) Wcx) ;
            #if GB_IS_PLUS_FC32_MONOID
            float  *GB_RESTRICT Hx_real = (float *) Hx ;
            float  *GB_RESTRICT Hx_imag = Hx_real + 1 ;
            #elif GB_IS_PLUS_FC64_MONOID
            double *GB_RESTRICT Hx_real = (double *) Hx ;
            double *GB_RESTRICT Hx_imag = Hx_real + 1 ;
            #endif

            //------------------------------------------------------------------
            // C<#M>(i1:i2,j) += reduce (W (i2:i2, wstart:wend))
            //------------------------------------------------------------------

            for (int64_t w = wstart ; w < wend ; w++)
            {

                //--------------------------------------------------------------
                // C<#M>(i1:i2,j) += W (i1:i2,w)
                //--------------------------------------------------------------
            
                int64_t pW_start = w * cvlen ;      // pointer to W (:,w)

                for (int64_t i = istart ; i < iend ; i++)
                {

                    //----------------------------------------------------------
                    // get pointer and bitmap C(i,j) and W(i,w)
                    //----------------------------------------------------------

                    int64_t pW = pW_start + i ;     // pointer to W(i,w)
                    if (Wf [pW] == 0) continue ;    // skip if not present
                    int64_t pC = pC_start + i ;     // pointer to C(i,j)
                    int8_t cb = Cb [pC] ;           // bitmap status of C(i,j)

                    //----------------------------------------------------------
                    // M(i,j) already checked, but adjust Cb if M is sparse
                    //----------------------------------------------------------

                    #if GB_MASK_IS_SPARSE_OR_HYPER
                    { 
                        // M is sparse or hypersparse
                        cb = (cb & 1) ;
                    }
                    #endif

                    //----------------------------------------------------------
                    // C(i,j) += W (i,w)
                    //----------------------------------------------------------

                    if (cb == 0)
                    { 
                        // C(i,j) = W(i,w)
                        #if GB_IS_ANY_PAIR_SEMIRING
                        Cx [pC] = GB_CTYPE_CAST (1, 0) ;        // C(i,j) = 1
                        #else
                        GB_CIJ_GATHER (pC, pW) ;
                        #endif
                        Cb [pC] = keep ;
                        task_cnvals++ ;
                    }
                    else
                    { 
                        // C(i,j) += W(i,w)
                        GB_CIJ_GATHER_UPDATE (pC, pW) ;
                    }
                }
            }
            cnvals += task_cnvals ;
        }
    }
}

