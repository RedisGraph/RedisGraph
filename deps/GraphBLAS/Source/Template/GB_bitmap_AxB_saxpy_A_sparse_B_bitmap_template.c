//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy_A_sparse_B_bitmap: C<#M>+=A*B, C bitmap, M any format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is bitmap or full. A is hyper/sparse, B is bitmap/full.

// if C is bitmap: no accumulator is used

// if C is full: C += A*B is computed with the accumulator identical to
// the monoid

{

    if (use_coarse_tasks)
    {

        //----------------------------------------------------------------------
        // C<#M> += A*B using coarse tasks
        //----------------------------------------------------------------------

        // number of columns in the workspace for each task
        #define GB_PANEL_SIZE 4

        if (B_iso)
        { 
            // No special cases needed.  GB_GETB handles the B iso case.
        }

        //----------------------------------------------------------------------
        // allocate workspace for each task
        //----------------------------------------------------------------------

        GB_WERK_PUSH (H_slice, ntasks, int64_t) ;
        if (H_slice == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        int64_t hwork = 0 ;
        int tid ;
        for (tid = 0 ; tid < ntasks ; tid++)
        {
            int64_t jstart, jend ;
            GB_PARTITION (jstart, jend, bvdim, tid, ntasks) ;
            int64_t jtask = jend - jstart ;
            int64_t jpanel = GB_IMIN (jtask, GB_PANEL_SIZE) ;
            H_slice [tid] = hwork ;
            #if ( !GB_C_IS_BITMAP )
            // bitmap case always needs Hx workspace; full case only needs it
            // if jpanel > 1
            if (jpanel > 1)
            #endif
            { 
                hwork += jpanel ;
            }
        }

        //----------------------------------------------------------------------

        int64_t cvlenx = (GB_IS_ANY_PAIR_SEMIRING ? 0 : cvlen) * GB_CSIZE ;
        #if GB_C_IS_BITMAP
        Wf  = GB_MALLOC_WORK (hwork * cvlen, int8_t, &Wf_size) ;
        #endif
        Wcx = GB_MALLOC_WORK (hwork * cvlenx, GB_void, &Wcx_size) ;
        if ((GB_C_IS_BITMAP && Wf == NULL) || Wcx == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // C<#M> += A*B
        //----------------------------------------------------------------------

        #if GB_C_IS_BITMAP
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:cnvals)
        #else
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        #endif
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            //------------------------------------------------------------------
            // determine the vectors of B and C for this coarse task
            //------------------------------------------------------------------

            int64_t jstart, jend ;
            GB_PARTITION (jstart, jend, bvdim, tid, ntasks) ;
            int64_t jtask = jend - jstart ;
            int64_t jpanel = GB_IMIN (jtask, GB_PANEL_SIZE) ;
            #if GB_C_IS_BITMAP
            int64_t task_cnvals = 0 ;
            #endif

            //------------------------------------------------------------------
            // get the workspace for this task
            //------------------------------------------------------------------

            // Hf and Hx workspace to compute the panel of C
            #if GB_C_IS_BITMAP
            int8_t *restrict Hf = Wf + (H_slice [tid] * cvlen) ;
            #endif
            #if ( !GB_IS_ANY_PAIR_SEMIRING )
            GB_CTYPE *restrict Hx = (GB_CTYPE *) (Wcx + H_slice [tid] * cvlenx);
            #endif

            //------------------------------------------------------------------
            // clear the panel
            //------------------------------------------------------------------

            #if GB_C_IS_BITMAP
            memset (Hf, 0, jpanel * cvlen) ;
            #endif

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
                // G = B(:,j1:j2-1), of size bvlen-by-np, in column major order
                //--------------------------------------------------------------

                int8_t *restrict Gb = (int8_t *) (Bb + (j1 * bvlen)) ;
                #if ( !GB_IS_ANY_PAIR_SEMIRING )
                GB_BTYPE *restrict Gx = (GB_BTYPE *)
                     (((GB_void *) (B->x)) +
                       (B_iso ? 0 : ((j1 * bvlen) * GB_BSIZE))) ;
                #endif

                //--------------------------------------------------------------
                // clear the panel H to compute C(:,j1:j2-1)
                //--------------------------------------------------------------

                #if ( !GB_C_IS_BITMAP )
                if (np == 1)
                { 
                    // Make H an alias to C(:,j1)
                    int64_t j = j1 ;
                    int64_t pC_start = j * cvlen ;    // get pointer to C(:,j)
                    Hx = Cx + pC_start ;
                }
                else
                { 
                    // Hx = identity
                    int64_t nc = np * cvlen ;
                    #if GB_HAS_IDENTITY_BYTE
                        memset (Hx, GB_IDENTITY_BYTE, nc * GB_CSIZE) ;
                    #else
                        for (int64_t i = 0 ; i < nc ; i++)
                        { 
                            Hx [i] = GB_IDENTITY ;
                        }
                    #endif
                }
                #endif

                #if GB_IS_PLUS_FC32_MONOID
                float  *restrict Hx_real = (float *) Hx ;
                float  *restrict Hx_imag = Hx_real + 1 ;
                #elif GB_IS_PLUS_FC64_MONOID
                double *restrict Hx_real = (double *) Hx ;
                double *restrict Hx_imag = Hx_real + 1 ;
                #endif

                //--------------------------------------------------------------
                // H += A*G for one panel
                //--------------------------------------------------------------

                #undef GB_B_kj_PRESENT
                #if GB_B_IS_BITMAP
                #define GB_B_kj_PRESENT(b) b
                #else
                #define GB_B_kj_PRESENT(b) 1
                #endif

                #undef GB_MULT_A_ik_G_kj
                #if GB_IS_PAIR_MULTIPLIER
                    // t = A(i,k) * B (k,j) is already #defined as 1
                    #define GB_MULT_A_ik_G_kj(gkj,jj)
                #else
                    // t = A(i,k) * B (k,j)
                    #define GB_MULT_A_ik_G_kj(gkj,jj)                       \
                        GB_CIJ_DECLARE (t) ;                                \
                        GB_MULT (t, aik, gkj, i, k, j1 + jj)
                #endif

                #undef GB_HX_COMPUTE
                #if GB_C_IS_BITMAP
                    #define GB_HX_COMPUTE(gkj,gb,jj)                        \
                    {                                                       \
                        /* H (i,jj) += A(i,k) * B(k,j) */                   \
                        if (GB_B_kj_PRESENT (gb))                           \
                        {                                                   \
                            /* t = A(i,k) * B (k,j) */                      \
                            GB_MULT_A_ik_G_kj (gkj, jj) ;                   \
                            if (Hf [pH+jj] == 0)                            \
                            {                                               \
                                /* H(i,jj) is a new entry */                \
                                GB_HX_WRITE (pH+jj, t) ; /* Hx(i,jj)=t */   \
                                Hf [pH+jj] = 1 ;                            \
                            }                                               \
                            else                                            \
                            {                                               \
                                /* H(i,jj) is already present */            \
                                /* Hx(i,jj)+=t */                           \
                                GB_HX_UPDATE (pH+jj, t) ;                   \
                            }                                               \
                        }                                                   \
                    }
                #else
                    #define GB_HX_COMPUTE(gkj,gb,jj)                        \
                    {                                                       \
                        /* H (i,jj) += A(i,k) * B(k,j) */                   \
                        if (GB_B_kj_PRESENT (gb))                           \
                        {                                                   \
                            /* t = A(i,k) * B (k,j) */                      \
                            GB_MULT_A_ik_G_kj (gkj, jj) ;                   \
                            /* Hx(i,jj)+=t */                               \
                            GB_HX_UPDATE (pH+jj, t) ;                       \
                        }                                                   \
                    }
                #endif

                switch (np)
                {

                    case 4 : 

                        for (int64_t kA = 0 ; kA < anvec ; kA++)
                        {
                            // get A(:,k)
                            const int64_t k = GBH (Ah, kA) ;
                            // get B(k,j1:j2-1)
                            #if GB_B_IS_BITMAP
                            const int8_t gb0 = Gb [k          ] ;
                            const int8_t gb1 = Gb [k +   bvlen] ;
                            const int8_t gb2 = Gb [k + 2*bvlen] ;
                            const int8_t gb3 = Gb [k + 3*bvlen] ;
                            if (!(gb0 || gb1 || gb2 || gb3)) continue ;
                            #endif
                            GB_GETB (gk0, Gx, k          , B_iso) ;
                            GB_GETB (gk1, Gx, k +   bvlen, B_iso) ;
                            GB_GETB (gk2, Gx, k + 2*bvlen, B_iso) ;
                            GB_GETB (gk3, Gx, k + 3*bvlen, B_iso) ;
                            // H += A(:,k)*B(k,j1:j2-1)
                            const int64_t pA_end = Ap [kA+1] ;
                            for (int64_t pA = Ap [kA] ; pA < pA_end ; pA++)
                            { 
                                const int64_t i = Ai [pA] ;
                                const int64_t pH = i * 4 ;
                                GB_GETA (aik, Ax, pA, A_iso) ;
                                GB_HX_COMPUTE (gk0, gb0, 0) ;
                                GB_HX_COMPUTE (gk1, gb1, 1) ;
                                GB_HX_COMPUTE (gk2, gb2, 2) ;
                                GB_HX_COMPUTE (gk3, gb3, 3) ;
                            }
                        }
                        break ;

                    case 3 : 

                        for (int64_t kA = 0 ; kA < anvec ; kA++)
                        {
                            // get A(:,k)
                            const int64_t k = GBH (Ah, kA) ;
                            // get B(k,j1:j2-1)
                            #if GB_B_IS_BITMAP
                            const int8_t gb0 = Gb [k          ] ;
                            const int8_t gb1 = Gb [k +   bvlen] ;
                            const int8_t gb2 = Gb [k + 2*bvlen] ;
                            if (!(gb0 || gb1 || gb2)) continue ;
                            #endif
                            GB_GETB (gk0, Gx, k          , B_iso) ;
                            GB_GETB (gk1, Gx, k +   bvlen, B_iso) ;
                            GB_GETB (gk2, Gx, k + 2*bvlen, B_iso) ;
                            // H += A(:,k)*B(k,j1:j2-1)
                            const int64_t pA_end = Ap [kA+1] ;
                            for (int64_t pA = Ap [kA] ; pA < pA_end ; pA++)
                            { 
                                const int64_t i = Ai [pA] ;
                                const int64_t pH = i * 3 ;
                                GB_GETA (aik, Ax, pA, A_iso) ;
                                GB_HX_COMPUTE (gk0, gb0, 0) ;
                                GB_HX_COMPUTE (gk1, gb1, 1) ;
                                GB_HX_COMPUTE (gk2, gb2, 2) ;
                            }
                        }
                        break ;

                    case 2 : 

                        for (int64_t kA = 0 ; kA < anvec ; kA++)
                        {
                            // get A(:,k)
                            const int64_t k = GBH (Ah, kA) ;
                            // get B(k,j1:j2-1)
                            #if GB_B_IS_BITMAP
                            const int8_t gb0 = Gb [k          ] ;
                            const int8_t gb1 = Gb [k +   bvlen] ;
                            if (!(gb0 || gb1)) continue ;
                            #endif
                            // H += A(:,k)*B(k,j1:j2-1)
                            GB_GETB (gk0, Gx, k          , B_iso) ;
                            GB_GETB (gk1, Gx, k +   bvlen, B_iso) ;
                            const int64_t pA_end = Ap [kA+1] ;
                            for (int64_t pA = Ap [kA] ; pA < pA_end ; pA++)
                            { 
                                const int64_t i = Ai [pA] ;
                                const int64_t pH = i * 2 ;
                                GB_GETA (aik, Ax, pA, A_iso) ;
                                GB_HX_COMPUTE (gk0, gb0, 0) ;
                                GB_HX_COMPUTE (gk1, gb1, 1) ;
                            }
                        }
                        break ;

                    case 1 : 

                        for (int64_t kA = 0 ; kA < anvec ; kA++)
                        {
                            // get A(:,k)
                            const int64_t k = GBH (Ah, kA) ;
                            // get B(k,j1:j2-1) where j1 == j2-1
                            #if GB_B_IS_BITMAP
                            const int8_t gb0 = Gb [k] ;
                            if (!gb0) continue ;
                            #endif
                            // H += A(:,k)*B(k,j1:j2-1)
                            GB_GETB (gk0, Gx, k, B_iso) ;
                            const int64_t pA_end = Ap [kA+1] ;
                            for (int64_t pA = Ap [kA] ; pA < pA_end ; pA++)
                            { 
                                const int64_t i = Ai [pA] ;
                                const int64_t pH = i ;
                                GB_GETA (aik, Ax, pA, A_iso) ;
                                GB_HX_COMPUTE (gk0, 1, 0) ;
                            }
                        }
                        break ;

                    default:;
                }

                #undef GB_HX_COMPUTE
                #undef GB_B_kj_PRESENT
                #undef GB_MULT_A_ik_G_kj

                //--------------------------------------------------------------
                // C<#M>(:,j1:j2-1) = H
                //--------------------------------------------------------------

                #if ( !GB_C_IS_BITMAP )
                if (np == 1)
                { 
                    // Hx is already aliased to Cx; no more work to do
                    continue ;
                }
                #endif

                for (int64_t jj = 0 ; jj < np ; jj++)
                {

                    //----------------------------------------------------------
                    // C<#M>(:,j) = H (:,jj)
                    //----------------------------------------------------------

                    int64_t j = j1 + jj ;
                    int64_t pC_start = j * cvlen ;  // get pointer to C(:,j)

                    for (int64_t i = 0 ; i < cvlen ; i++)
                    {
                        int64_t pC = pC_start + i ;     // pointer to C(i,j)
                        int64_t pH = i * np + jj ;      // pointer to H(i,jj)
                        #if GB_C_IS_BITMAP
                        if (!Hf [pH]) continue ;
                        Hf [pH] = 0 ;                   // clear the panel
                        int8_t cb = Cb [pC] ;
                        #endif

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

                        #if GB_C_IS_BITMAP
                        if (cb == 0)
                        { 
                            // C(i,j) = H(i,jj)
                            GB_CIJ_GATHER (pC, pH) ;
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
                        #else
                        { 
                            // C(i,j) = H(i,jj)
                            GB_CIJ_GATHER_UPDATE (pC, pH) ;
                        }
                        #endif
                    }
                }
            }
            #if GB_C_IS_BITMAP
            cnvals += task_cnvals ;
            #endif
        }

        #undef GB_PANEL_SIZE

    }
    else if (use_atomics)
    {

        //----------------------------------------------------------------------
        // C<#M> += A*B using fine tasks and atomics
        //----------------------------------------------------------------------

        if (B_iso)
        { 
            // No special cases needed.  GB_GET_B_kj (bkj = B(k,j))
            // handles the B iso case.
        }

        int tid ;
        #if GB_C_IS_BITMAP
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:cnvals)
        #else
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        #endif
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
            int64_t pC_start = j * cvlen ;      // pointer to C(:,j)
            GB_GET_T_FOR_SECONDJ ;              // t = j or j+1 for SECONDJ*
            #if GB_C_IS_BITMAP
            int64_t task_cnvals = 0 ;
            #endif

            // for Hx Gustavason workspace: use C(:,j) in-place:
            #if ( !GB_IS_ANY_PAIR_SEMIRING )
            GB_CTYPE *restrict Hx = (GB_CTYPE *)
                (((GB_void *) Cx) + (pC_start * GB_CSIZE)) ;
            #endif
            #if GB_IS_PLUS_FC32_MONOID || GB_IS_ANY_FC32_MONOID
            float  *restrict Hx_real = (float *) Hx ;
            float  *restrict Hx_imag = Hx_real + 1 ;
            #elif GB_IS_PLUS_FC64_MONOID || GB_IS_ANY_FC64_MONOID
            double *restrict Hx_real = (double *) Hx ;
            double *restrict Hx_imag = Hx_real + 1 ;
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
                #if GB_B_IS_BITMAP
                if (!GBB (Bb, pB)) continue ;   
                #endif
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

                    //----------------------------------------------------------
                    // C<#M>(i,j) += A(i,k) * B(k,j)
                    //----------------------------------------------------------

                    #if ( !GB_C_IS_BITMAP )
                    { 

                        //------------------------------------------------------
                        // C is full: the monoid is always atomic
                        //------------------------------------------------------

                        GB_MULT_A_ik_B_kj ;     // t = A(i,k) * B(k,j)
                        GB_ATOMIC_UPDATE_HX (i, t) ;    // C(i,j) += t

                    }
                    #elif GB_MASK_IS_SPARSE_OR_HYPER
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

                        int8_t cb ;
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
                            GB_ATOMIC_WRITE_HX (i, t) ;     // C(i,j) = t
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

                        int8_t cb ;
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
                            GB_ATOMIC_WRITE_HX (i, t) ;     // C(i,j) = t
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
            #if GB_C_IS_BITMAP
            cnvals += task_cnvals ;
            #endif
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

        if (B_iso)
        { 
            // No special cases needed.  GB_GET_B_kj (bkj = B(k,j))
            // handles the B iso case.
        }

        //----------------------------------------------------------------------
        // allocate workspace
        //----------------------------------------------------------------------

        size_t workspace = cvlen * ntasks ;
        size_t cxsize = (GB_IS_ANY_PAIR_SEMIRING) ? 0 : GB_CSIZE ;
        #if GB_C_IS_BITMAP
        Wf  = GB_MALLOC_WORK (workspace, int8_t, &Wf_size) ;
        #endif
        Wcx = GB_MALLOC_WORK (workspace * cxsize, GB_void, &Wcx_size) ;
        if ((GB_C_IS_BITMAP && Wf == NULL) || Wcx == NULL)
        { 
            // out of memory
            GB_FREE_ALL ;
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
            int64_t pC_start = j * cvlen ;      // pointer to C(:,j), for bitmap
            int64_t pW_start = tid * cvlen ;    // pointer to W(:,tid)
            GB_GET_T_FOR_SECONDJ ;              // t = j or j+1 for SECONDJ*
            #if GB_C_IS_BITMAP
            int64_t task_cnvals = 0 ;
            #endif

            // for Hf and Hx Gustavason workspace: use W(:,tid):
            #if GB_C_IS_BITMAP
            int8_t *restrict Hf = Wf + pW_start ;
            #endif
            #if ( !GB_IS_ANY_PAIR_SEMIRING )
            GB_CTYPE *restrict Hx = (GB_CTYPE *) (Wcx + (pW_start * cxsize)) ;
            #endif
            #if GB_IS_PLUS_FC32_MONOID
            float  *restrict Hx_real = (float *) Hx ;
            float  *restrict Hx_imag = Hx_real + 1 ;
            #elif GB_IS_PLUS_FC64_MONOID
            double *restrict Hx_real = (double *) Hx ;
            double *restrict Hx_imag = Hx_real + 1 ;
            #endif

            //------------------------------------------------------------------
            // clear the panel
            //------------------------------------------------------------------

            #if GB_C_IS_BITMAP
            { 
                memset (Hf, 0, cvlen) ;
            }
            #else
            { 
                // set Hx to identity
                #if GB_HAS_IDENTITY_BYTE
                    memset (Hx, GB_IDENTITY_BYTE, cvlen * GB_CSIZE) ;
                #else
                    for (int64_t i = 0 ; i < cvlen ; i++)
                    { 
                        Hx [i] = GB_IDENTITY ;
                    }
                #endif
            }
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
                #if GB_B_IS_BITMAP
                if (!GBB (Bb, pB)) continue ;   
                #endif
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
                        Hf [i] = 1 ;
                    }
                    #else
                    {
                        GB_MULT_A_ik_B_kj ;         // t = A(i,k)*B(k,j)
                        #if GB_C_IS_BITMAP
                        if (Hf [i] == 0)
                        { 
                            // W(i) is a new entry
                            GB_HX_WRITE (i, t) ;    // Hx(i) = t
                            Hf [i] = 1 ;
                        }
                        else
                        #endif
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

        #if GB_C_IS_BITMAP
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:cnvals)
        #else
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        #endif
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
            #if GB_C_IS_BITMAP
            int64_t task_cnvals = 0 ;
            #endif

            // Hx = (typecasted) Wcx workspace, use Wf as-is
            #if ( !GB_IS_ANY_PAIR_SEMIRING )
            GB_CTYPE *restrict Hx = ((GB_CTYPE *) Wcx) ;
            #endif
            #if GB_IS_PLUS_FC32_MONOID
            float  *restrict Hx_real = (float *) Hx ;
            float  *restrict Hx_imag = Hx_real + 1 ;
            #elif GB_IS_PLUS_FC64_MONOID
            double *restrict Hx_real = (double *) Hx ;
            double *restrict Hx_imag = Hx_real + 1 ;
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
                    #if GB_C_IS_BITMAP
                    if (Wf [pW] == 0) continue ;    // skip if not present
                    #endif
                    int64_t pC = pC_start + i ;     // pointer to C(i,j)
                    #if GB_C_IS_BITMAP
                    int8_t cb = Cb [pC] ;           // bitmap status of C(i,j)
                    #endif

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

                    #if GB_C_IS_BITMAP
                    if (cb == 0)
                    { 
                        // C(i,j) = W(i,w)
                        GB_CIJ_GATHER (pC, pW) ;
                        Cb [pC] = keep ;
                        task_cnvals++ ;
                    }
                    else
                    #endif
                    { 
                        // C(i,j) += W(i,w)
                        GB_CIJ_GATHER_UPDATE (pC, pW) ;
                    }
                }
            }
            #if GB_C_IS_BITMAP
            cnvals += task_cnvals ;
            #endif
        }
    }
}

