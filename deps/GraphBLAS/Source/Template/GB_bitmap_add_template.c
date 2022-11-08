//------------------------------------------------------------------------------
// GB_bitmap_add_template: C = A+B, C<M>=A+B, and C<!M>=A+B, C bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is bitmap.  The mask M can have any sparsity structure, and is efficient
// to apply (all methods are asymptotically optimal).  All cases (no M, M, !M)
// are handled.  The values of A, B, and C are not accessed if C is iso,
// in which case GB_ISO_ADD is #defined' by the #including file.

{

    // TODO: the input C can be modified in-place, if it is also bitmap
    int64_t cnvals = 0 ;

    if (M == NULL)
    {

        //----------------------------------------------------------------------
        // M is not present
        //----------------------------------------------------------------------

        //      ------------------------------------------
        //      C       =           A       +       B
        //      ------------------------------------------
        //      bitmap  .           sparse          bitmap
        //      bitmap  .           bitmap          sparse
        //      bitmap  .           bitmap          bitmap

        ASSERT (A_is_bitmap || B_is_bitmap) ;
        ASSERT (!A_is_full) ;
        ASSERT (!B_is_full) ;

        if (A_is_bitmap && B_is_bitmap)
        {

            //------------------------------------------------------------------
            // Method21: C, A, and B are all bitmap
            //------------------------------------------------------------------

            int tid ;
            #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
                reduction(+:cnvals)
            for (tid = 0 ; tid < C_nthreads ; tid++)
            {
                int64_t pstart, pend, task_cnvals = 0 ;
                GB_PARTITION (pstart, pend, cnz, tid, C_nthreads) ;
                for (int64_t p = pstart ; p < pend ; p++)
                {
                    #ifdef GB_ISO_ADD
                    int8_t c = Ab [p] || Bb [p] ;
                    #else
                    int8_t c = 0 ;
                    if (Ab [p] && Bb [p])
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        GB_LOAD_A (aij, Ax, p, A_iso) ;
                        GB_LOAD_B (bij, Bx, p, B_iso) ;
                        GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
                        c = 1 ;
                    }
                    else if (Bb [p])
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = alpha + B(i,j)
                            GB_LOAD_B (bij, Bx, p, B_iso) ;
                            GB_BINOP (GB_CX (p), alpha_scalar, bij,
                                p % vlen, p / vlen) ;
                        }
                        #else
                        { 
                            // C (i,j) = B (i,j)
                            GB_COPY_B_TO_C (GB_CX (p), Bx, p, B_iso) ;
                        }
                        #endif
                        c = 1 ;
                    }
                    else if (Ab [p])
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = A(i,j) + beta
                            GB_LOAD_A (aij, Ax, p, A_iso) ;
                            GB_BINOP (GB_CX (p), aij, beta_scalar,
                                p % vlen, p / vlen) ;
                        }
                        #else
                        { 
                            // C (i,j) = A (i,j)
                            GB_COPY_A_TO_C (GB_CX (p), Ax, p, A_iso) ;
                        }
                        #endif
                        c = 1 ;
                    }
                    #endif
                    Cb [p] = c ;
                    task_cnvals += c ;
                }
                cnvals += task_cnvals ;
            }

        }
        else if (A_is_bitmap)
        {

            //------------------------------------------------------------------
            // Method22: C and A are bitmap; B is sparse or hypersparse
            //------------------------------------------------------------------

            #ifdef GB_ISO_ADD
                GB_memcpy (Cb, Ab, cnz, C_nthreads) ;
            #else
                int64_t p ;
                #pragma omp parallel for num_threads(C_nthreads) \
                    schedule(static)
                for (p = 0 ; p < cnz ; p++)
                { 
                    int8_t a = Ab [p] ;
                    if (a)
                    { 
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = A(i,j) + beta
                            GB_LOAD_A (aij, Ax, p, A_iso) ;
                            GB_BINOP (GB_CX (p), aij, beta_scalar,
                                p % vlen, p / vlen) ;
                        }
                        #else
                        { 
                            // C (i,j) = A (i,j)
                            GB_COPY_A_TO_C (GB_CX (p), Ax, p, A_iso) ;
                        }
                        #endif
                    }
                    Cb [p] = a ;
                }
            #endif
            cnvals = A->nvals ;

            GB_SLICE_MATRIX (B, 8, chunk) ;

            #pragma omp parallel for num_threads(B_nthreads) \
                schedule(dynamic,1) reduction(+:cnvals)
            for (taskid = 0 ; taskid < B_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Bslice [taskid] ;
                int64_t klast  = klast_Bslice  [taskid] ;
                int64_t task_cnvals = 0 ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of B(:,k) for this task
                    int64_t j = GBH (Bh, k) ;
                    int64_t pB_start, pB_end ;
                    GB_get_pA (&pB_start, &pB_end, taskid, k, kfirst,
                        klast, pstart_Bslice, Bp, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over B(:,j), the kth vector of B
                    for (int64_t pB = pB_start ; pB < pB_end ; pB++)
                    {
                        int64_t i = Bi [pB] ;
                        int64_t p = pC_start + i ;
                        if (Cb [p])
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            #ifndef GB_ISO_ADD
                            GB_LOAD_A (aij, Ax, p , A_iso) ;
                            GB_LOAD_B (bij, Bx, pB, B_iso) ;
                            GB_BINOP (GB_CX (p), aij, bij, i, j) ;
                            #endif
                        }
                        else
                        { 
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (p), alpha_scalar, bij, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (p), Bx, pB, B_iso) ;
                            }
                            #endif
                            #endif
                            Cb [p] = 1 ;
                            task_cnvals++ ;
                        }
                    }
                }
                cnvals += task_cnvals ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // Method23: C and B are bitmap; A is sparse or hypersparse
            //------------------------------------------------------------------

            #ifdef GB_ISO_ADD
                GB_memcpy (Cb, Bb, cnz, C_nthreads) ;
            #else
                int64_t p ;
                #pragma omp parallel for num_threads(C_nthreads) \
                    schedule(static)
                for (p = 0 ; p < cnz ; p++)
                { 
                    int8_t b = Bb [p] ;
                    if (b)
                    {
                        #ifdef GB_EWISEUNION
                        { 
                            // C (i,j) = alpha + B(i,j)
                            GB_LOAD_B (bij, Bx, p, B_iso) ;
                            GB_BINOP (GB_CX (p), alpha_scalar, bij,
                                p % vlen, p / vlen) ;
                        }
                        #else
                        { 
                            // C (i,j) = B (i,j)
                            GB_COPY_B_TO_C (GB_CX (p), Bx, p, B_iso) ;
                        }
                        #endif
                    }
                    Cb [p] = b ;
                }
            #endif

            cnvals = B->nvals ;

            GB_SLICE_MATRIX (A, 8, chunk) ;

            #pragma omp parallel for num_threads(A_nthreads) \
                schedule(dynamic,1) reduction(+:cnvals)
            for (taskid = 0 ; taskid < A_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Aslice [taskid] ;
                int64_t klast  = klast_Aslice  [taskid] ;
                int64_t task_cnvals = 0 ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of A(:,k) for this task
                    int64_t j = GBH (Ah, k) ;
                    int64_t pA_start, pA_end ;
                    GB_get_pA (&pA_start, &pA_end, taskid, k, kfirst,
                        klast, pstart_Aslice, Ap, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over A(:,j), the kth vector of A
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;
                        int64_t p = pC_start + i ;
                        if (Cb [p])
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            #ifndef GB_ISO_ADD
                            GB_LOAD_A (aij, Ax, pA, A_iso) ;
                            GB_LOAD_B (bij, Bx, p , B_iso) ;
                            GB_BINOP (GB_CX (p), aij, bij, i, j) ;
                            #endif
                        }
                        else
                        { 
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_BINOP (GB_CX (p), aij, beta_scalar, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (p), Ax, pA, A_iso) ;
                            }
                            #endif
                            #endif
                            Cb [p] = 1 ;
                            task_cnvals++ ;
                        }
                    }
                }
                cnvals += task_cnvals ;
            }
        }

    }
    else if (M_is_sparse_or_hyper)
    { 

        //----------------------------------------------------------------------
        // C is bitmap, M is sparse or hyper and complemented
        //----------------------------------------------------------------------

        //      ------------------------------------------
        //      C     <!M> =        A       +       B
        //      ------------------------------------------
        //      bitmap  sparse      sparse          bitmap
        //      bitmap  sparse      sparse          full  
        //      bitmap  sparse      bitmap          sparse
        //      bitmap  sparse      bitmap          bitmap
        //      bitmap  sparse      bitmap          full  
        //      bitmap  sparse      full            sparse
        //      bitmap  sparse      full            bitmap
        //      bitmap  sparse      full            full  

        // M is sparse and complemented.  If M is sparse and not
        // complemented, then C is constructed as sparse, not bitmap.
        ASSERT (Mask_comp) ;

        // C(i,j) = A(i,j) + B(i,j) can only be computed where M(i,j) is
        // not present in the sparse pattern of M, and where it is present
        // but equal to zero.

        //----------------------------------------------------------------------
        // scatter M into the C bitmap
        //----------------------------------------------------------------------

        GB_SLICE_MATRIX (M, 8, chunk) ;

        #pragma omp parallel for num_threads(M_nthreads) schedule(dynamic,1)
        for (taskid = 0 ; taskid < M_ntasks ; taskid++)
        {
            int64_t kfirst = kfirst_Mslice [taskid] ;
            int64_t klast  = klast_Mslice  [taskid] ;
            for (int64_t k = kfirst ; k <= klast ; k++)
            {
                // find the part of M(:,k) for this task
                int64_t j = GBH (Mh, k) ;
                int64_t pM_start, pM_end ;
                GB_get_pA (&pM_start, &pM_end, taskid, k, kfirst,
                    klast, pstart_Mslice, Mp, vlen) ;
                int64_t pC_start = j * vlen ;
                // traverse over M(:,j), the kth vector of M
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                {
                    // mark C(i,j) if M(i,j) is true
                    bool mij = GB_mcast (Mx, pM, msize) ;
                    if (mij)
                    { 
                        int64_t i = Mi [pM] ;
                        int64_t p = pC_start + i ;
                        Cb [p] = 2 ;
                    }
                }
            }
        }

        // C(i,j) has been marked, in Cb, with the value 2 where M(i,j)=1.
        // These positions will not be computed in C(i,j).  C(i,j) can only
        // be modified where Cb [p] is zero.

        //----------------------------------------------------------------------
        // compute C<!M>=A+B using the mask scattered in C
        //----------------------------------------------------------------------

        bool M_cleared = false ;

        if ((A_is_bitmap || A_is_full) && (B_is_bitmap || B_is_full))
        {

            //------------------------------------------------------------------
            // Method24(!M,sparse): C is bitmap, both A and B are bitmap or full
            //------------------------------------------------------------------

            int tid ;
            #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
                reduction(+:cnvals)
            for (tid = 0 ; tid < C_nthreads ; tid++)
            {
                int64_t pstart, pend, task_cnvals = 0 ;
                GB_PARTITION (pstart, pend, cnz, tid, C_nthreads) ;
                for (int64_t p = pstart ; p < pend ; p++)
                {
                    int8_t c = Cb [p] ;
                    if (c == 0)
                    {
                        // M(i,j) is zero, so C(i,j) can be computed
                        int8_t a = GBB (Ab, p) ;
                        int8_t b = GBB (Bb, p) ;
                        #ifdef GB_ISO_ADD
                        c = a || b ;
                        #else
                        if (a && b)
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            GB_LOAD_A (aij, Ax, p, A_iso) ;
                            GB_LOAD_B (bij, Bx, p, B_iso) ;
                            GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
                            c = 1 ;
                        }
                        else if (b)
                        { 
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, p, B_iso) ;
                                GB_BINOP (GB_CX (p), alpha_scalar, bij,
                                    p % vlen, p / vlen) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (p), Bx, p, B_iso) ;
                            }
                            #endif
                            c = 1 ;
                        }
                        else if (a)
                        { 
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, p, A_iso) ;
                                GB_BINOP (GB_CX (p), aij, beta_scalar,
                                    p % vlen, p / vlen) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (p), Ax, p, A_iso) ;
                            }
                            #endif
                            c = 1 ;
                        }
                        #endif
                        Cb [p] = c ;
                        task_cnvals += c ;
                    }
                    else
                    { 
                        // M(i,j) == 1, so C(i,j) is not computed
                        Cb [p] = 0 ;
                    }
                }
                cnvals += task_cnvals ;
            }
            M_cleared = true ;      // M has also been cleared from C

        }
        else if (A_is_bitmap || A_is_full)
        {

            //------------------------------------------------------------------
            // Method25(!M,sparse): C bitmap, A bitmap or full, B sparse/hyper
            //------------------------------------------------------------------

            int tid ;
            #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
                reduction(+:cnvals)
            for (tid = 0 ; tid < C_nthreads ; tid++)
            {
                int64_t pstart, pend, task_cnvals = 0 ;
                GB_PARTITION (pstart, pend, cnz, tid, C_nthreads) ;
                for (int64_t p = pstart ; p < pend ; p++)
                {
                    if (Cb [p] == 0)
                    { 
                        int8_t a = GBB (Ab, p) ;
                        #ifndef GB_ISO_ADD
                        if (a)
                        {
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, p, A_iso) ;
                                GB_BINOP (GB_CX (p), aij, beta_scalar,
                                    p % vlen, p / vlen) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (p), Ax, p, A_iso) ;
                            }
                            #endif
                        }
                        #endif
                        Cb [p] = a ;
                        task_cnvals += a ;
                    }
                }
                cnvals += task_cnvals ;
            }

            GB_SLICE_MATRIX (B, 8, chunk) ;

            #pragma omp parallel for num_threads(B_nthreads) \
                schedule(dynamic,1) reduction(+:cnvals)
            for (taskid = 0 ; taskid < B_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Bslice [taskid] ;
                int64_t klast  = klast_Bslice  [taskid] ;
                int64_t task_cnvals = 0 ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of B(:,k) for this task
                    int64_t j = GBH (Bh, k) ;
                    int64_t pB_start, pB_end ;
                    GB_get_pA (&pB_start, &pB_end, taskid, k, kfirst,
                        klast, pstart_Bslice, Bp, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over B(:,j), the kth vector of B
                    for (int64_t pB = pB_start ; pB < pB_end ; pB++)
                    {
                        int64_t i = Bi [pB] ;
                        int64_t p = pC_start + i ;
                        int8_t c = Cb [p] ;
                        if (c == 1)
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            #ifndef GB_ISO_ADD
                            GB_LOAD_A (aij, Ax, p , A_iso) ;
                            GB_LOAD_B (bij, Bx, pB, B_iso) ;
                            GB_BINOP (GB_CX (p), aij, bij, i, j) ;
                            #endif
                        }
                        else if (c == 0)
                        { 
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            {
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (p), alpha_scalar, bij, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (p), Bx, pB, B_iso) ;
                            }
                            #endif
                            #endif
                            Cb [p] = 1 ;
                            task_cnvals++ ;
                        }
                    }
                }
                cnvals += task_cnvals ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // Method26: C bitmap, A sparse or hypersparse, B bitmap or full
            //------------------------------------------------------------------

            int tid ;
            #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
                reduction(+:cnvals)
            for (tid = 0 ; tid < C_nthreads ; tid++)
            {
                int64_t pstart, pend, task_cnvals = 0 ;
                GB_PARTITION (pstart, pend, cnz, tid, C_nthreads) ;
                for (int64_t p = pstart ; p < pend ; p++)
                {
                    if (Cb [p] == 0)
                    { 
                        int8_t b = GBB (Bb, p) ;
                        #ifndef GB_ISO_ADD
                        if (b)
                        {
                            #ifdef GB_EWISEUNION
                            {
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, p, B_iso) ;
                                GB_BINOP (GB_CX (p), alpha_scalar, bij,
                                    p % vlen, p / vlen) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (p), Bx, p, B_iso) ;
                            }
                            #endif
                        }
                        #endif
                        Cb [p] = b ;
                        task_cnvals += b ;
                    }
                }
                cnvals += task_cnvals ;
            }

            GB_SLICE_MATRIX (A, 8, chunk) ;

            #pragma omp parallel for num_threads(A_nthreads) \
                schedule(dynamic,1) reduction(+:cnvals)
            for (taskid = 0 ; taskid < A_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Aslice [taskid] ;
                int64_t klast  = klast_Aslice  [taskid] ;
                int64_t task_cnvals = 0 ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of A(:,k) for this task
                    int64_t j = GBH (Ah, k) ;
                    int64_t pA_start, pA_end ;
                    GB_get_pA (&pA_start, &pA_end, taskid, k, kfirst,
                        klast, pstart_Aslice, Ap, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over A(:,j), the kth vector of A
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;
                        int64_t p = pC_start + i ;
                        int8_t c = Cb [p] ;
                        if (c == 1)
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            #ifndef GB_ISO_ADD
                            GB_LOAD_A (aij, Ax, pA, A_iso) ;
                            GB_LOAD_B (bij, Bx, p , B_iso) ;
                            GB_BINOP (GB_CX (p), aij, bij, i, j) ;
                            #endif
                        }
                        else if (c == 0)
                        { 
                            #ifndef GB_ISO_ADD
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_BINOP (GB_CX (p), aij, beta_scalar, i, j) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (p), Ax, pA, A_iso) ;
                            }
                            #endif
                            #endif
                            Cb [p] = 1 ;
                            task_cnvals++ ;
                        }
                    }
                }
                cnvals += task_cnvals ;
            }
        }

        //---------------------------------------------------------------------
        // clear M from C
        //---------------------------------------------------------------------

        if (!M_cleared)
        {
            // This step is required if either A or B are sparse/hyper (if
            // one is sparse/hyper, the other must be bitmap).  It requires
            // an extra pass over the mask M, so this might be slower than
            // postponing the application of the mask, and doing it later.

            #pragma omp parallel for num_threads(M_nthreads) schedule(dynamic,1)
            for (taskid = 0 ; taskid < M_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Mslice [taskid] ;
                int64_t klast  = klast_Mslice  [taskid] ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of M(:,k) for this task
                    int64_t j = GBH (Mh, k) ;
                    int64_t pM_start, pM_end ;
                    GB_get_pA (&pM_start, &pM_end, taskid, k, kfirst,
                        klast, pstart_Mslice, Mp, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over M(:,j), the kth vector of M
                    for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                    {
                        // mark C(i,j) if M(i,j) is true
                        bool mij = GB_mcast (Mx, pM, msize) ;
                        if (mij)
                        { 
                            int64_t i = Mi [pM] ;
                            int64_t p = pC_start + i ;
                            Cb [p] = 0 ;
                        }
                    }
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C is bitmap; M is bitmap or full
        //----------------------------------------------------------------------

        //      ------------------------------------------
        //      C      <M> =        A       +       B
        //      ------------------------------------------
        //      bitmap  bitmap      sparse          bitmap
        //      bitmap  bitmap      sparse          full  
        //      bitmap  bitmap      bitmap          sparse
        //      bitmap  bitmap      bitmap          bitmap
        //      bitmap  bitmap      bitmap          full  
        //      bitmap  bitmap      full            sparse
        //      bitmap  bitmap      full            bitmap
        //      bitmap  bitmap      full            full  

        //      ------------------------------------------
        //      C      <M> =        A       +       B
        //      ------------------------------------------
        //      bitmap  full        sparse          bitmap
        //      bitmap  full        sparse          full  
        //      bitmap  full        bitmap          sparse
        //      bitmap  full        bitmap          bitmap
        //      bitmap  full        bitmap          full  
        //      bitmap  full        full            sparse
        //      bitmap  full        full            bitmap
        //      bitmap  full        full            full  

        //      ------------------------------------------
        //      C     <!M> =        A       +       B
        //      ------------------------------------------
        //      bitmap  bitmap      sparse          sparse
        //      bitmap  bitmap      sparse          bitmap
        //      bitmap  bitmap      sparse          full  
        //      bitmap  bitmap      bitmap          sparse
        //      bitmap  bitmap      bitmap          bitmap
        //      bitmap  bitmap      bitmap          full  
        //      bitmap  bitmap      full            sparse
        //      bitmap  bitmap      full            bitmap
        //      bitmap  bitmap      full            full  

        //      ------------------------------------------
        //      C     <!M> =        A       +       B
        //      ------------------------------------------
        //      bitmap  full        sparse          sparse
        //      bitmap  full        sparse          bitmap
        //      bitmap  full        sparse          full  
        //      bitmap  full        bitmap          sparse
        //      bitmap  full        bitmap          bitmap
        //      bitmap  full        bitmap          full  
        //      bitmap  full        full            sparse
        //      bitmap  full        full            bitmap
        //      bitmap  full        full            full  


        ASSERT (M_is_bitmap || M_is_full) ;
        ASSERT (A_is_bitmap || A_is_full || B_is_bitmap || B_is_full) ;

        #undef  GB_GET_MIJ     
        #define GB_GET_MIJ(p)                                           \
            bool mij = GBB (Mb, p) && GB_mcast (Mx, p, msize) ;         \
            if (Mask_comp) mij = !mij ;

        if ((A_is_bitmap || A_is_full) && (B_is_bitmap || B_is_full))
        {

            //------------------------------------------------------------------
            // Method27: C is bitmap; M, A, and B are bitmap or full
            //------------------------------------------------------------------

            int tid ;
            #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
                reduction(+:cnvals)
            for (tid = 0 ; tid < C_nthreads ; tid++)
            {
                int64_t pstart, pend, task_cnvals = 0 ;
                GB_PARTITION (pstart, pend, cnz, tid, C_nthreads) ;
                for (int64_t p = pstart ; p < pend ; p++)
                {
                    GB_GET_MIJ (p) ;
                    if (mij)
                    {
                        // M(i,j) is true, so C(i,j) can be computed
                        int8_t a = GBB (Ab, p) ;
                        int8_t b = GBB (Bb, p) ;
                        #ifdef GB_ISO_ADD
                        int8_t c = a || b ;
                        #else
                        int8_t c = 0 ;
                        if (a && b)
                        { 
                            // C (i,j) = A (i,j) + B (i,j)
                            GB_LOAD_A (aij, Ax, p, A_iso) ;
                            GB_LOAD_B (bij, Bx, p, B_iso) ;
                            GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
                            c = 1 ;
                        }
                        else if (b)
                        { 
                            #ifdef GB_EWISEUNION
                            {
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, p, B_iso) ;
                                GB_BINOP (GB_CX (p), alpha_scalar, bij,
                                    p % vlen, p / vlen) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (p), Bx, p, B_iso) ;
                            }
                            #endif
                            c = 1 ;
                        }
                        else if (a)
                        { 
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, p, A_iso) ;
                                GB_BINOP (GB_CX (p), aij, beta_scalar,
                                    p % vlen, p / vlen) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (p), Ax, p, A_iso) ;
                            }
                            #endif
                            c = 1 ;
                        }
                        #endif
                        Cb [p] = c ;
                        task_cnvals += c ;
                    }
                    else
                    { 
                        // M(i,j) == 1, so C(i,j) is not computed
                        Cb [p] = 0 ;
                    }
                }
                cnvals += task_cnvals ;
            }

        }
        else if (A_is_bitmap || A_is_full)
        {

            //------------------------------------------------------------------
            // Method28: C bitmap; M and A bitmap or full; B sparse or hyper
            //------------------------------------------------------------------

            int tid ;
            #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
                reduction(+:cnvals)
            for (tid = 0 ; tid < C_nthreads ; tid++)
            {
                int64_t pstart, pend, task_cnvals = 0 ;
                GB_PARTITION (pstart, pend, cnz, tid, C_nthreads) ;
                for (int64_t p = pstart ; p < pend ; p++)
                {
                    GB_GET_MIJ (p) ;
                    if (mij)
                    { 
                        int8_t a = GBB (Ab, p) ;
                        #ifndef GB_ISO_ADD
                        if (a)
                        {
                            #ifdef GB_EWISEUNION
                            { 
                                // C (i,j) = A(i,j) + beta
                                GB_LOAD_A (aij, Ax, p, A_iso) ;
                                GB_BINOP (GB_CX (p), aij, beta_scalar,
                                    p % vlen, p / vlen) ;
                            }
                            #else
                            { 
                                // C (i,j) = A (i,j)
                                GB_COPY_A_TO_C (GB_CX (p), Ax, p, A_iso) ;
                            }
                            #endif
                        }
                        #endif
                        Cb [p] = a ;
                        task_cnvals += a ;
                    }
                    else
                    { 
                        Cb [p] = 0 ;
                    }
                }
                cnvals += task_cnvals ;
            }

            GB_SLICE_MATRIX (B, 8, chunk) ;

            #pragma omp parallel for num_threads(B_nthreads) \
                schedule(dynamic,1) reduction(+:cnvals)
            for (taskid = 0 ; taskid < B_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Bslice [taskid] ;
                int64_t klast  = klast_Bslice  [taskid] ;
                int64_t task_cnvals = 0 ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of B(:,k) for this task
                    int64_t j = GBH (Bh, k) ;
                    int64_t pB_start, pB_end ;
                    GB_get_pA (&pB_start, &pB_end, taskid, k, kfirst,
                        klast, pstart_Bslice, Bp, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over B(:,j), the kth vector of B
                    for (int64_t pB = pB_start ; pB < pB_end ; pB++)
                    {
                        int64_t i = Bi [pB] ;
                        int64_t p = pC_start + i ;
                        GB_GET_MIJ (p) ;
                        if (mij)
                        {
                            int8_t c = Cb [p] ;
                            if (c == 1)
                            { 
                                // C (i,j) = A (i,j) + B (i,j)
                                #ifndef GB_ISO_ADD
                                GB_LOAD_A (aij, Ax, p , A_iso) ;
                                GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                GB_BINOP (GB_CX (p), aij, bij, i, j) ;
                                #endif
                            }
                            else
                            { 
                                #ifndef GB_ISO_ADD
                                #ifdef GB_EWISEUNION
                                {
                                    // C (i,j) = alpha + B(i,j)
                                    GB_LOAD_B (bij, Bx, pB, B_iso) ;
                                    GB_BINOP (GB_CX (p), alpha_scalar, bij,
                                        i, j) ;
                                }
                                #else
                                { 
                                    // C (i,j) = B (i,j)
                                    GB_COPY_B_TO_C (GB_CX (p), Bx, pB, B_iso) ;
                                }
                                #endif
                                #endif
                                Cb [p] = 1 ;
                                task_cnvals++ ;
                            }
                        }
                    }
                }
                cnvals += task_cnvals ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // Method29: C bitmap; M and B bitmap or full; A sparse or hyper
            //------------------------------------------------------------------

            int tid ;
            #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
                reduction(+:cnvals)
            for (tid = 0 ; tid < C_nthreads ; tid++)
            {
                int64_t pstart, pend, task_cnvals = 0 ;
                GB_PARTITION (pstart, pend, cnz, tid, C_nthreads) ;
                for (int64_t p = pstart ; p < pend ; p++)
                {
                    GB_GET_MIJ (p) ;
                    if (mij)
                    { 
                        int8_t b = GBB (Bb, p) ;
                        #ifndef GB_ISO_ADD
                        if (b)
                        {
                            #ifdef GB_EWISEUNION
                            {
                                // C (i,j) = alpha + B(i,j)
                                GB_LOAD_B (bij, Bx, p, B_iso) ;
                                GB_BINOP (GB_CX (p), alpha_scalar, bij,
                                    p % vlen, p / vlen) ;
                            }
                            #else
                            { 
                                // C (i,j) = B (i,j)
                                GB_COPY_B_TO_C (GB_CX (p), Bx, p, B_iso) ;
                            }
                            #endif
                        }
                        #endif
                        Cb [p] = b ;
                        task_cnvals += b ;
                    }
                    else
                    { 
                        Cb [p] = 0 ;
                    }
                }
                cnvals += task_cnvals ;
            }

            GB_SLICE_MATRIX (A, 8, chunk) ;

            #pragma omp parallel for num_threads(A_nthreads) \
                schedule(dynamic,1) reduction(+:cnvals)
            for (taskid = 0 ; taskid < A_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Aslice [taskid] ;
                int64_t klast  = klast_Aslice  [taskid] ;
                int64_t task_cnvals = 0 ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of A(:,k) for this task
                    int64_t j = GBH (Ah, k) ;
                    int64_t pA_start, pA_end ;
                    GB_get_pA (&pA_start, &pA_end, taskid, k, kfirst,
                        klast, pstart_Aslice, Ap, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over A(:,j), the kth vector of A
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    {
                        int64_t i = Ai [pA] ;
                        int64_t p = pC_start + i ;
                        GB_GET_MIJ (p) ;
                        if (mij)
                        {
                            int8_t c = Cb [p] ;
                            if (c == 1)
                            { 
                                // C (i,j) = A (i,j) + B (i,j)
                                #ifndef GB_ISO_ADD
                                GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                GB_LOAD_B (bij, Bx, p , B_iso) ;
                                GB_BINOP (GB_CX (p), aij, bij, i, j) ;
                                #endif
                            }
                            else
                            { 
                                #ifndef GB_ISO_ADD
                                #ifdef GB_EWISEUNION
                                { 
                                    // C (i,j) = A(i,j) + beta
                                    GB_LOAD_A (aij, Ax, pA, A_iso) ;
                                    GB_BINOP (GB_CX (p), aij, beta_scalar,
                                        i, j) ;
                                }
                                #else
                                { 
                                    // C (i,j) = A (i,j)
                                    GB_COPY_A_TO_C (GB_CX (p), Ax, pA, A_iso) ;
                                }
                                #endif
                                #endif
                                Cb [p] = 1 ;
                                task_cnvals++ ;
                            }
                        }
                    }
                }
                cnvals += task_cnvals ;
            }
        }
    }

    C->nvals = cnvals ;
}

