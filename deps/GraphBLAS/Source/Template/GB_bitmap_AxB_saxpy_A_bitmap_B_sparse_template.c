//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy_A_bitmap_B_sparse: C<#M>+=A*B, C bitmap, M any format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is bitmap, A is bitmap or full, B is sparse or hypersparse.
// M has any format.

{

    //--------------------------------------------------------------------------
    // allocate workspace for each task
    //--------------------------------------------------------------------------

    // imeta = total number of rows of A and H in all panels
    int64_t imeta = naslice * GB_PANEL_SIZE ;

    // number of entries in one panel of G for A.
    #if GB_HAS_BITMAP_MULTADD && !GB_IS_ANY_PAIR_SEMIRING
    // Always load the A panel into G, since Ax [pA] has uninitialized values
    // where Ab [pA] == 0.  The GB_BITMAP_MULTADD update will access these
    // values, and they must be initialized.
    const bool load_apanel = true ;
    #else
    // only load the A panel into G if it consists of more than one panel
    const bool load_apanel = (avlen > GB_PANEL_SIZE) ;
    #endif
    // Each panel of G is GB_PANEL_SIZE-by-avdim, held by column.
    int64_t apanel_size = load_apanel ? (GB_PANEL_SIZE * avdim) : 0 ;
    int64_t afpanel_size = GB_A_IS_BITMAP  ? (apanel_size) : 0 ;
    int64_t axpanel_size = A_is_pattern ? 0 : (apanel_size * GB_ASIZE) ;

    // each panel of H is GB_PANEL_SIZE-by-bnvec, held by column; note that
    // H has bnvec vectors, not bvdim.  The C bitmap has bvdim vectors,
    // and bnvec <= bvdim if B is hypersparse.
    int64_t hpanel_size = GB_PANEL_SIZE * bnvec ;

    //--------------------------------------------------------------------------
    // allocate the panels
    //--------------------------------------------------------------------------

    // The G panels are not needed if A would fit into a single panel.
    // In that case A is used in place and not copied into G.

    int64_t wafsize = naslice * afpanel_size ;
    int64_t waxsize = naslice * axpanel_size ;
    int64_t wcsize  = naslice * hpanel_size ;
    int64_t wcxsize = GB_IS_ANY_PAIR_SEMIRING ? 0 : (wcsize * GB_CSIZE) ;
    Wf = GB_MALLOC (wafsize + wcsize, int8_t) ;
    Wax = GB_MALLOC (waxsize, GB_void) ;
    Wcx = GB_MALLOC (wcxsize, GB_void) ;
    if (Wf == NULL || Wax == NULL || Wcx == NULL)
    { 
        // out of memory
        GB_FREE_WORK ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // initialize the panels
    //--------------------------------------------------------------------------

    // for all semirings: set the bitmaps Gb and Hf to zero
    GB_memset (Wf,  0, wafsize + wcsize, nthreads_max) ;

    #if GB_HAS_BITMAP_MULTADD && !GB_IS_ANY_PAIR_SEMIRING
    { 
        // Initialize the Hx workspace to identity, if this semiring has a
        // concise bitmap multiply-add expression.  For the any_pair semiring,
        // the numerical values are not needed so Hx is not allocated.
        #if GB_HAS_IDENTITY_BYTE
            // the identity value can be assigned via memset
            GB_memset (Wcx, GB_IDENTITY_BYTE, wcxsize, nthreads_max) ;
        #else
            // an explicit loop is required to set Hx to identity
            // TODO: should each task initialize its own Hf and Hx,
            // and use a static schedule here and for H=G*B?
            GB_CTYPE *GB_RESTRICT Hx = (GB_CTYPE *) Wcx ;
            int64_t pH ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (pH = 0 ; pH < wcsize ; pH++)
            {
                Hx [pH] = GB_IDENTITY ;
            }
        #endif
    }
    #endif

    //--------------------------------------------------------------------------
    // C<#M>=A*B, one metapanel at a time
    //--------------------------------------------------------------------------

    int tid ;

    for (int64_t iouter = 0 ; iouter < avlen ; iouter += imeta)
    {

        //----------------------------------------------------------------------
        // C<#M>(metapanel,:) += A (metapanel,:)*B
        //----------------------------------------------------------------------

        // The rows in this metapanel are iouter:iouter+imeta-1.

        //----------------------------------------------------------------------
        // load the metapanel: G = A (iouter:iouter+imeta-1,:)
        //----------------------------------------------------------------------

        if ((GB_A_IS_BITMAP || !A_is_pattern) && load_apanel)
        {

            // Loading the panel into G keeps its storage order.  A is not
            // transposed when loaded into the G panels.  However, the leading
            // dimension is reduced.  A is avlen-by-avdim with a leading
            // dimension of avlen, which can be large.  G is np-by-avdim, with
            // np <= GB_PANEL_SIZE.  The loading of A into G can be skipped
            // if all of A can be used in-place.

            #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
            for (tid = 0 ; tid < ntasks ; tid++)
            {

                //--------------------------------------------------------------
                // get the panel for this task
                //--------------------------------------------------------------

                int a_tid = tid / nbslice ;
                int b_tid = tid % nbslice ;
                int64_t istart = iouter + a_tid     * GB_PANEL_SIZE ;
                int64_t iend   = iouter + (a_tid+1) * GB_PANEL_SIZE ;
                iend = GB_IMIN (iend, avlen) ;
                int64_t np = iend - istart ;
                if (np <= 0) continue ;
                int64_t kstart, kend ; 
                GB_PARTITION (kstart, kend, avdim, b_tid, nbslice) ;
                int8_t   *GB_RESTRICT Gb = Wf  + (a_tid * afpanel_size) ;
                GB_ATYPE *GB_RESTRICT Gx = (GB_ATYPE *)
                    (Wax + (a_tid * axpanel_size)) ;

                //--------------------------------------------------------------
                // load A for this panel
                //--------------------------------------------------------------

                #if ( GB_A_IS_BITMAP )
                {

                    //----------------------------------------------------------
                    // A is bitmap
                    //----------------------------------------------------------

                    if (!A_is_pattern)
                    {
                        // load Ab and Ax into Gb and Gx
                        for (int64_t k = kstart ; k < kend ; k++)
                        {
                            for (int64_t ii = 0 ; ii < np ; ii++)
                            {
                                // Gb (ii,k) = Ab (istart+ii,k)
                                const int64_t pG = ii + k*np ;
                                const int64_t pA = istart + ii + k*avlen ;
                                const int8_t gb = Ab [pA] ;
                                Gb [pG] = gb ;
                                if (gb)
                                { 
                                    // Gx (ii,k) = Ax (istart+ii,k)
                                    GB_LOADA (Gx, pG, Ax, pA) ;
                                }
                                #if GB_HAS_BITMAP_MULTADD
                                else
                                { 
                                    // Gx (ii,k) = 0
                                    Gx [pG] = GB_ATYPE_CAST (0, 0) ;
                                }
                                #endif
                            }
                        }
                    }
                    else
                    {
                        // just load the Ab bitmap into Gb, not the values
                        for (int64_t k = kstart ; k < kend ; k++)
                        {
                            for (int64_t ii = 0 ; ii < np ; ii++)
                            { 
                                // Gb (ii,k) = Ab (istart+ii,k)
                                const int64_t pG = ii + k*np ;
                                const int64_t pA = istart + ii + k*avlen ;
                                Gb [pG] = Ab [pA] ;
                            }
                        }
                    }

                }
                #else
                {

                    //----------------------------------------------------------
                    // A is full
                    //----------------------------------------------------------

                    if (!A_is_pattern)
                    {
                        for (int64_t k = kstart ; k < kend ; k++)
                        {
                            for (int64_t ii = 0 ; ii < np ; ii++)
                            { 
                                // Gx (ii,k) = Ax (istart+ii,k)
                                const int64_t pG = ii + k*np ;
                                const int64_t pA = istart + ii + k*avlen ;
                                GB_LOADA (Gx, pG, Ax, pA) ;
                            }
                        }
                    }
                }
                #endif
            }
        }

        //----------------------------------------------------------------------
        // H = G*B
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            //------------------------------------------------------------------
            // get the panel of H and G for this task
            //------------------------------------------------------------------

            int a_tid = tid / nbslice ;
            int b_tid = tid % nbslice ;
            int64_t istart = iouter + a_tid     * GB_PANEL_SIZE ;
            int64_t iend   = iouter + (a_tid+1) * GB_PANEL_SIZE ;
            iend = GB_IMIN (iend, avlen) ;
            int64_t np = iend - istart ;
            if (np <= 0) continue ;

            const int8_t   *GB_RESTRICT Gb ;
            const GB_ATYPE *GB_RESTRICT Gx ;

            if (load_apanel)
            { 
                // A has been loaded into the G panel
                Gb = Wf  + (a_tid * afpanel_size) ;
                Gx = (GB_ATYPE *) (Wax + (a_tid * axpanel_size)) ;
            }
            else
            { 
                // use A in-place
                Gb = Ab ;
                Gx = (GB_ATYPE *) Ax ;
            }

            int8_t   *GB_RESTRICT Hf = Wf  + (a_tid * hpanel_size) + wafsize ;
            GB_CTYPE *GB_RESTRICT Hx = (GB_CTYPE *)
                (Wcx + (a_tid * hpanel_size) * GB_CSIZE) ;
            GB_XINIT ;  // for plus, bor, band, and bxor monoids only

            //------------------------------------------------------------------
            // H_panel (:,kfirst:klast-1) = G_panel * B (:, kfirst:klast-1)
            //------------------------------------------------------------------

            int64_t kfirst = B_slice [b_tid] ;
            int64_t klast = B_slice [b_tid + 1] ;
            for (int64_t kk = kfirst ; kk < klast ; kk++)
            {

                //--------------------------------------------------------------
                // H_panel (:,kk) = G_panel * B (:,kk)
                //--------------------------------------------------------------

                // H and B are indexed in the compact space kk = 0:bnvec-1,
                // not by the names j = 0:bvdim-1.  When B is sparse, these are
                // the same.  If B is hypersparse, j is Bh [kk].  However, j is
                // needed for the SECONDJ and SECONDJ1 multipliers.

                int64_t j = GBH (Bh, kk) ;
                int64_t pB = Bp [kk] ;
                int64_t pB_end = Bp [kk+1] ;
                int64_t pH = kk * np ;
                #if GB_IS_SECONDJ_MULTIPLIER
                    // t = j or j+1 for SECONDJ and SECONDJ1 multipliers
                    GB_CIJ_DECLARE (t) ;
                    GB_MULT (t, ignore, ignore, ignore, ignore, j) ;
                #endif

                #undef GB_MULT_G_iik_B_kj
                #if GB_IS_PAIR_MULTIPLIER
                    // t = G(ii,k) * B(k,j) is always equal to 1
                    #define GB_MULT_G_iik_B_kj(ii)
                #elif ( GB_IS_FIRSTJ_MULTIPLIER || GB_IS_SECONDJ_MULTIPLIER )
                    // t is already defined for these multipliers
                    #define GB_MULT_G_iik_B_kj(ii)
                #else
                    // t = G(ii,k) * B(k,j)
                    #define GB_MULT_G_iik_B_kj(ii)                          \
                        GB_GETA (giik, Gx, pG + ii) ;                       \
                        GB_CIJ_DECLARE (t) ;                                \
                        GB_MULT (t, giik, bkj, istart + ii, k, j)
                #endif

                for ( ; pB < pB_end ; pB++)
                {
                    int64_t k = Bi [pB] ;       // get B(k,j)
                    int64_t pG = k * np ;       // get G(:,k)
                    GB_GET_B_kj ;               // bkj = B(k,j)
                    GB_XLOAD (bkj) ;            // X [1] = bkj (plus_times only)
                    // H_panel (:,j) = G_panel (:,k) * B(k,j)
                    for (int64_t ii = 0 ; ii < np ; ii++)
                    { 
                        #if GB_HAS_BITMAP_MULTADD
                        { 
                            // if (Gb (ii,k))
                            //      if (Hf (ii,j) == 0)
                            //          Hx (ii,j) = G (ii,k) * B(k,j) ;
                            //          Hf (ii,j) = 1
                            //      else
                            //          Hx (ii,j) += G (ii,k) * B(k,j) ;
                            #if GB_IS_FIRSTI_MULTIPLIER
                            int64_t i = istart + ii ;
                            #endif
                            #if GB_A_IS_BITMAP
                                GB_BITMAP_MULTADD (
                                    Hf [pH+ii], Hx [pH+ii],
                                    Gb [pG+ii], Gx [pG+ii], bkj) ;
                            #else
                                GB_BITMAP_MULTADD (
                                    Hf [pH+ii], Hx [pH+ii],
                                    1,          Gx [pG+ii], bkj) ;
                            #endif
                        }
                        #else
                        { 
                            #if GB_A_IS_BITMAP
                            if (Gb [pG+ii])
                            #endif
                            {
                                // t = G(ii,k) * B(k,j)
                                GB_MULT_G_iik_B_kj (ii) ;
                                if (Hf [pH+ii] == 0)
                                { 
                                    // H (ii,j) is a new entry
                                    GB_HX_WRITE (pH+ii, t) ;    // Hx (ii,j)=t
                                    Hf [pH+ii] = 1 ;
                                }
                                else
                                { 
                                    // H (ii,j) is already present
                                    GB_HX_UPDATE (pH+ii, t) ;   // Hx (ii,j)+=t
                                }
                            }
                        }
                        #endif
                    }
                }
                #undef GB_MULT_G_iik_B_kj
            }
        }

        //----------------------------------------------------------------------
        // C (metapanel,:) += H
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
            reduction(+:cnvals)
        for (tid = 0 ; tid < ntasks ; tid++)
        {

            //------------------------------------------------------------------
            // get the panel of H and G for this task
            //------------------------------------------------------------------

            int a_tid = tid / nbslice ;
            int b_tid = tid % nbslice ;
            int64_t istart = iouter + a_tid     * GB_PANEL_SIZE ;
            int64_t iend   = iouter + (a_tid+1) * GB_PANEL_SIZE ;
            iend = GB_IMIN (iend, avlen) ;
            int64_t np = iend - istart ;
            if (np <= 0) continue ;
            int64_t task_cnvals = 0 ;

            int64_t kstart, kend ; 
            GB_PARTITION (kstart, kend, bnvec, b_tid, nbslice) ;

            int8_t   *GB_RESTRICT Hf = Wf  + (a_tid * hpanel_size) + wafsize ;
            GB_CTYPE *GB_RESTRICT Hx = (GB_CTYPE *)
                (Wcx + (a_tid * hpanel_size) * GB_CSIZE) ;

            //------------------------------------------------------------------
            // C<#M>(metapanel,j1:j2-1) += H (:,kstart:kend-1)
            //------------------------------------------------------------------

            // If B is hypersparse, the kk-th vector of H is the jth vector
            // of C, where j = Bh [kk].

            for (int64_t kk = kstart ; kk < kend ; kk++)
            {
                int64_t j = GBH (Bh, kk) ;      // j is the range j1:j2-1
                int64_t pC_start = istart + j * avlen ; // get C(istart,j)
                int64_t pH_start = kk * np ;            // get H(:,kk)

                for (int64_t ii = 0 ; ii < np ; ii++)
                {
                    int64_t pC = pC_start + ii ;    // get C(i,j)
                    int64_t pH = pH_start + ii ;    // get H(ii,kk)
                    if (!Hf [pH]) continue ;
                    Hf [pH] = 0 ;                   // clear the panel
                    int8_t cb = Cb [pC] ;

                    //----------------------------------------------------------
                    // check M(i,j)
                    //----------------------------------------------------------

                    #undef GB_IF_MIJ
                    #if GB_MASK_IS_SPARSE_OR_HYPER

                        // M is sparse or hypersparse
                        bool mij = ((cb & 2) != 0) ^ Mask_comp ;
                        cb = (cb & 1) ;
                        #define GB_IF_MIJ if (mij)

                    #elif GB_MASK_IS_BITMAP_OR_FULL

                        // M is bitmap or full
                        GB_GET_M_ij (pC) ;
                        mij = mij ^ Mask_comp ;
                        #define GB_IF_MIJ if (mij)

                    #else

                        #define GB_IF_MIJ

                    #endif

                    //----------------------------------------------------------
                    // C(i,j) += H(ii,kk)
                    //----------------------------------------------------------

                    GB_IF_MIJ
                    {
                        if (cb == 0)
                        { 
                            // C(i,j) = H(ii,kk)
                            #if GB_IS_ANY_PAIR_SEMIRING
                            Cx [pC] = GB_CTYPE_CAST (1,0) ; // C(i,j) = 1
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
                            // C(i,j) += H(ii,kk)
                            GB_CIJ_GATHER_UPDATE (pC, pH) ;
                        }
                    }

                    //----------------------------------------------------------
                    // clear the panel
                    //----------------------------------------------------------

                    #if GB_HAS_BITMAP_MULTADD && !GB_IS_ANY_PAIR_SEMIRING
                    { 
                        // H(ii,kk) = identity
                        Hx [pH] = GB_IDENTITY ;
                    }
                    #endif
                }
            }
            cnvals += task_cnvals ;
        }
    }
}

#undef GB_IF_MIJ

