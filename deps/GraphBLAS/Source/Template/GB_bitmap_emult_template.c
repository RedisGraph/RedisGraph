//------------------------------------------------------------------------------
// GB_bitmap_emult_template: C = A.*B, C<M>=A.*B, and C<!M>=A.*B, C bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is bitmap.  A and B are bitmap or full.  M depends on the method

{

    //--------------------------------------------------------------------------
    // get C, A, and B
    //--------------------------------------------------------------------------

    const int8_t  *restrict Ab = A->b ;
    const int8_t  *restrict Bb = B->b ;
    const int64_t vlen = A->vlen ;

    ASSERT (GB_IS_BITMAP (A) || GB_IS_FULL (A) || GB_as_if_full (A)) ;
    ASSERT (GB_IS_BITMAP (B) || GB_IS_FULL (A) || GB_as_if_full (B)) ;

    const bool A_iso = A->iso ;
    const bool B_iso = B->iso ;

    int8_t *restrict Cb = C->b ;
    const int64_t cnz = GB_nnz_held (C) ;

    #ifdef GB_ISO_EMULT
    ASSERT (C->iso) ;
    #else
    ASSERT (!C->iso) ;
    ASSERT (!(A_iso && B_iso)) ;    // one of A or B can be iso, but not both
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    const GB_BTYPE *restrict Bx = (GB_BTYPE *) B->x ;
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif

    //--------------------------------------------------------------------------
    // C=A.*B, C<M>=A.*B, or C<!M>=A.*B: C is bitmap
    //--------------------------------------------------------------------------

    // TODO modify this method so it can modify C in-place, and also use the
    // accum operator.
    int64_t cnvals = 0 ;

    if (ewise_method == GB_EMULT_METHOD5)
    {

        //----------------------------------------------------------------------
        // Method5: C is bitmap, M is not present
        //----------------------------------------------------------------------

        //      ------------------------------------------
        //      C       =           A       .*      B
        //      ------------------------------------------
        //      bitmap  .           bitmap          bitmap  (method: 5)
        //      bitmap  .           bitmap          full    (method: 5)
        //      bitmap  .           full            bitmap  (method: 5)

        int tid ;
        #pragma omp parallel for num_threads(C_nthreads) schedule(static) \
            reduction(+:cnvals)
        for (tid = 0 ; tid < C_nthreads ; tid++)
        {
            int64_t pstart, pend, task_cnvals = 0 ;
            GB_PARTITION (pstart, pend, cnz, tid, C_nthreads) ;
            for (int64_t p = pstart ; p < pend ; p++)
            {
                if (GBB (Ab, p) && GBB (Bb,p))
                { 
                    // C (i,j) = A (i,j) + B (i,j)
                    #ifndef GB_ISO_EMULT
                    GB_GETA (aij, Ax, p, A_iso) ;
                    GB_GETB (bij, Bx, p, B_iso) ;
                    GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
                    #endif
                    Cb [p] = 1 ;
                    task_cnvals++ ;
                }
            }
            cnvals += task_cnvals ;
        }

    }
    else if (ewise_method == GB_EMULT_METHOD6)
    {

        //----------------------------------------------------------------------
        // Method6: C is bitmap, !M is sparse or hyper
        //----------------------------------------------------------------------

        //      ------------------------------------------
        //      C       <!M>=       A       .*      B
        //      ------------------------------------------
        //      bitmap  sparse      bitmap          bitmap  (method: 6)
        //      bitmap  sparse      bitmap          full    (method: 6)
        //      bitmap  sparse      full            bitmap  (method: 6)

        // M is sparse and complemented.  If M is sparse and not
        // complemented, then C is constructed as sparse, not bitmap.
        ASSERT (M != NULL) ;
        ASSERT (Mask_comp) ;
        ASSERT (GB_IS_SPARSE (M) || GB_IS_HYPERSPARSE (M)) ;

        // C(i,j) = A(i,j) .* B(i,j) can only be computed where M(i,j) is
        // not present in the sparse pattern of M, and where it is present
        // but equal to zero.

        //----------------------------------------------------------------------
        // scatter M into the C bitmap
        //----------------------------------------------------------------------

        GB_bitmap_M_scatter_whole (C, M, Mask_struct, GB_BITMAP_M_SCATTER_SET_2,
            M_ek_slicing, M_ntasks, M_nthreads, Context) ;

        // C(i,j) has been marked, in Cb, with the value 2 where M(i,j)=1.
        // These positions will not be computed in C(i,j).  C(i,j) can only
        // be modified where Cb [p] is zero.

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
                    // M(i,j) is zero, so C(i,j) can be computed
                    if (GBB (Ab, p) && GBB (Bb, p))
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        #ifndef GB_ISO_EMULT
                        GB_GETA (aij, Ax, p, A_iso) ;
                        GB_GETB (bij, Bx, p, B_iso) ;
                        GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
                        #endif
                        Cb [p] = 1 ;
                        task_cnvals++ ;
                    }
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
    else // if (ewise_method == GB_EMULT_METHOD7)
    {

        //----------------------------------------------------------------------
        // Method7: C is bitmap; M is bitmap or full
        //----------------------------------------------------------------------

        //      ------------------------------------------
        //      C      <M> =        A       .*      B
        //      ------------------------------------------
        //      bitmap  bitmap      bitmap          bitmap  (method: 7)
        //      bitmap  bitmap      bitmap          full    (method: 7)
        //      bitmap  bitmap      full            bitmap  (method: 7)

        //      ------------------------------------------
        //      C      <M> =        A       .*      B
        //      ------------------------------------------
        //      bitmap  full        bitmap          bitmap  (method: 7)
        //      bitmap  full        bitmap          full    (method: 7)
        //      bitmap  full        full            bitmap  (method: 7)

        //      ------------------------------------------
        //      C      <!M> =       A       .*      B
        //      ------------------------------------------
        //      bitmap  bitmap      bitmap          bitmap  (method: 7)
        //      bitmap  bitmap      bitmap          full    (method: 7)
        //      bitmap  bitmap      full            bitmap  (method: 7)

        //      ------------------------------------------
        //      C      <!M> =       A       .*      B
        //      ------------------------------------------
        //      bitmap  full        bitmap          bitmap  (method: 7)
        //      bitmap  full        bitmap          full    (method: 7)
        //      bitmap  full        full            bitmap  (method: 7)

        ASSERT (GB_IS_BITMAP (M) || GB_IS_FULL (M)) ;

        const int8_t  *restrict Mb = M->b ;
        const GB_void *restrict Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;
        size_t msize = M->type->size ;

        #undef  GB_GET_MIJ     
        #define GB_GET_MIJ(p)                                           \
            bool mij = GBB (Mb, p) && GB_mcast (Mx, p, msize) ;         \
            if (Mask_comp) mij = !mij ; /* TODO: use ^ */

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
                    if (GBB (Ab, p) && GBB (Bb, p))
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        #ifndef GB_ISO_EMULT
                        GB_GETA (aij, Ax, p, A_iso) ;
                        GB_GETB (bij, Bx, p, B_iso) ;
                        GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
                        #endif
                        Cb [p] = 1 ;
                        task_cnvals++ ;
                    }
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

    C->nvals = cnvals ;
}

