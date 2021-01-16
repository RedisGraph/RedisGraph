//------------------------------------------------------------------------------
// GB_AxB_dot4_template:  C+=A'*B via dot products, where C is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C+=A'*B where C is a dense matrix and computed in-place.  The monoid of the
// semiring matches the accum operator, and the type of C matches the ztype of
// accum.  That is, no typecasting can be done with C.

// The PAIR operator as the multiplier provides important special cases.

{

    //--------------------------------------------------------------------------
    // C += A'*B
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        //----------------------------------------------------------------------
        // get the task descriptor
        //----------------------------------------------------------------------

        const int a_tid = tid / nbslice ;
        const int b_tid = tid % nbslice ;
        const int64_t kA_start = A_slice [a_tid] ;
        const int64_t kA_end   = A_slice [a_tid+1] ;
        const int64_t kB_start = B_slice [b_tid] ;
        const int64_t kB_end   = B_slice [b_tid+1] ;

        //----------------------------------------------------------------------
        // C+=A'*B via dot products
        //----------------------------------------------------------------------

        for (int64_t kB = kB_start ; kB < kB_end ; kB++)
        {

            //------------------------------------------------------------------
            // get B(:,j) and C(:,j)
            //------------------------------------------------------------------

            #if GB_B_IS_HYPER
            const int64_t j = Bh [kB] ;
            #else
            const int64_t j = kB ;
            #endif

            const int64_t pC_start = j * cvlen ;

            #if ( GB_B_IS_HYPER || GB_B_IS_SPARSE )
                // B is sparse or hyper
                const int64_t pB_start = Bp [kB] ;
                const int64_t pB_end = Bp [kB+1] ;
                const int64_t bjnz = pB_end - pB_start ;
                if (bjnz == 0) continue ;
                #if ( GB_A_IS_HYPER || GB_A_IS_SPARSE )
                    // Both A and B are sparse/hyper; get first & last in B(:,j)
                    const int64_t ib_first = Bi [pB_start] ;
                    const int64_t ib_last  = Bi [pB_end-1] ;
                #endif
            #else
                // B is bitmap or full
                const int64_t pB_start = j * vlen ;
            #endif

            //------------------------------------------------------------------
            // C(:,j) += A'*B(:,j) where C is full
            //------------------------------------------------------------------

            for (int64_t kA = kA_start ; kA < kA_end ; kA++)
            {

                //--------------------------------------------------------------
                // get A(:,i)
                //--------------------------------------------------------------

                #if GB_A_IS_HYPER
                const int64_t i = Ah [kA] ;
                #else
                const int64_t i = kA ;
                #endif

                #if ( GB_A_IS_HYPER || GB_A_IS_SPARSE )
                // A is sparse or hyper
                int64_t pA = Ap [kA] ;
                const int64_t pA_end = Ap [kA+1] ;
                const int64_t ainz = pA_end - pA ;
                if (ainz == 0) continue ;
                #else
                // A is bitmap or full
                const int64_t pA = kA * vlen ;
                #endif

                //--------------------------------------------------------------
                // get C(i,j)
                //--------------------------------------------------------------

                GB_CIJ_DECLARE (cij) ;          // declare the cij scalar
                int64_t pC = i + pC_start ;     // C(i,j) is at Cx [pC]
                bool cij_updated = false ;

                //--------------------------------------------------------------
                // C(i,j) += A (:,i)*B(:,j): a single dot product
                //--------------------------------------------------------------

                int64_t pB = pB_start ;

                #if ( GB_A_IS_FULL && GB_B_IS_FULL )
                {

                    //----------------------------------------------------------
                    // both A and B are full
                    //----------------------------------------------------------

                    GB_GETC (cij, pC) ;             // cij = Cx [pC]
                    #if GB_IS_PAIR_MULTIPLIER
                    { 
                        #if GB_IS_ANY_MONOID
                        // ANY monoid: take the first entry found
                        GB_MULT (cij, ignore, ignore, 0, 0, 0) ;
                        #elif GB_IS_EQ_MONOID
                        // EQ_PAIR semiring
                        cij = (cij == 1) ;
                        #elif (GB_CTYPE_BITS > 0)
                        // PLUS, XOR monoids: A(:,i)'*B(:,j) is nnz(A(:,i)),
                        // for bool, 8-bit, 16-bit, or 32-bit integer
                        uint64_t t = ((uint64_t) cij) + vlen ;
                        cij = (GB_CTYPE) (t & GB_CTYPE_BITS) ;
                        #elif GB_IS_PLUS_FC32_MONOID
                        // PLUS monoid for float complex
                        cij = GxB_CMPLXF (crealf (cij) + (float) vlen, 0) ;
                        #elif GB_IS_PLUS_FC64_MONOID
                        // PLUS monoid for double complex
                        cij = GxB_CMPLX (creal (cij) + (double) vlen, 0) ;
                        #else
                        // PLUS monoid for float, double, or 64-bit integers 
                        cij += (GB_CTYPE) vlen ;
                        #endif
                    }
                    #else
                    {
                        GB_PRAGMA_SIMD_DOT (cij)
                        for (int64_t k = 0 ; k < vlen ; k++)
                        { 
                            GB_DOT_TERMINAL (cij) ;         // break if terminal
                            // cij += A(k,i) * B(k,j)
                            GB_GETA (aki, Ax, pA+k) ;       // aki = A(k,i)
                            GB_GETB (bkj, Bx, pB+k) ;       // bkj = B(k,j)
                            // cij += aki * bkj
                            GB_MULTADD (cij, aki, bkj, i, k, j) ;
                        }
                    }
                    #endif
                    GB_DOT_ALWAYS_SAVE_CIJ ;

                }
                #elif ( GB_A_IS_FULL && GB_B_IS_BITMAP )
                {

                    //----------------------------------------------------------
                    // A is full and B is bitmap
                    //----------------------------------------------------------

                    for (int64_t k = 0 ; k < vlen ; k++)
                    {
                        if (Bb [pB+k])
                        { 
                            GB_DOT (k, pA+k, pB+k) ;
                        }
                    }
                    GB_DOT_SAVE_CIJ ;

                }
                #elif ( GB_A_IS_FULL && ( GB_B_IS_SPARSE || GB_B_IS_HYPER ) )
                {

                    //----------------------------------------------------------
                    // A is full and B is sparse/hyper
                    //----------------------------------------------------------

                    GB_GETC (cij, pC) ;                 // cij = Cx [pC]
                    #if GB_IS_PAIR_MULTIPLIER
                    { 
                        #if GB_IS_ANY_MONOID
                        // ANY monoid: take the first entry found
                        // cij = 1, or CMPLX(1,0) for complex ANY
                        GB_MULT (cij, ignore, ignore, 0, 0, 0) ;
                        #elif GB_IS_EQ_MONOID
                        // EQ_PAIR semiring
                        cij = (cij == 1) ;
                        #elif (GB_CTYPE_BITS > 0)
                        // PLUS, XOR monoids: A(:,i)'*B(:,j) is nnz(A(:,i)),
                        // for bool, 8-bit, 16-bit, or 32-bit integer
                        uint64_t t = ((uint64_t) cij) + bjnz ;
                        cij = (GB_CTYPE) (t & GB_CTYPE_BITS) ;
                        #elif GB_IS_PLUS_FC32_MONOID
                        // PLUS monoid for float complex
                        cij = GxB_CMPLXF (crealf (cij) + (float) bjnz, 0) ;
                        #elif GB_IS_PLUS_FC64_MONOID
                        // PLUS monoid for double complex
                        cij = GxB_CMPLX (creal (cij) + (double) bjnz, 0) ;
                        #else
                        // PLUS monoid for float, double, or 64-bit integers
                        cij += (GB_CTYPE) bjnz ;
                        #endif
                    }
                    #else
                    {
                        GB_PRAGMA_SIMD_DOT (cij)
                        for (int64_t p = pB ; p < pB_end ; p++)
                        { 
                            GB_DOT_TERMINAL (cij) ;   // break if terminal
                            int64_t k = Bi [p] ;
                            // cij += A(k,i) * B(k,j)
                            GB_GETA (aki, Ax, pA+k) ;     // aki = A(k,i)
                            GB_GETB (bkj, Bx, p   ) ;     // bkj = B(k,j)
                            GB_MULTADD (cij, aki, bkj, i, k, j) ;
                        }
                    }
                    #endif
                    GB_DOT_ALWAYS_SAVE_CIJ ;

                }
                #elif ( GB_A_IS_BITMAP && GB_B_IS_FULL )
                {

                    //----------------------------------------------------------
                    // A is bitmap and B is full
                    //----------------------------------------------------------

                    for (int64_t k = 0 ; k < vlen ; k++)
                    {
                        if (Ab [pA+k])
                        { 
                            GB_DOT (k, pA+k, pB+k) ;
                        }
                    }
                    GB_DOT_SAVE_CIJ ;

                }
                #elif ( GB_A_IS_BITMAP && GB_B_IS_BITMAP )
                {

                    //----------------------------------------------------------
                    // both A and B are bitmap
                    //----------------------------------------------------------

                    for (int64_t k = 0 ; k < vlen ; k++)
                    {
                        if (Ab [pA+k] && Bb [pB+k])
                        { 
                            GB_DOT (k, pA+k, pB+k) ;
                        }
                    }
                    GB_DOT_SAVE_CIJ ;

                }
                #elif ( GB_A_IS_BITMAP && ( GB_B_IS_SPARSE || GB_B_IS_HYPER ) )
                {

                    //----------------------------------------------------------
                    // A is bitmap and B is sparse/hyper
                    //----------------------------------------------------------

                    for (int64_t p = pB ; p < pB_end ; p++)
                    {
                        int64_t k = Bi [p] ;
                        if (Ab [pA+k])
                        { 
                            GB_DOT (k, pA+k, p) ;
                        }
                    }
                    GB_DOT_SAVE_CIJ ;

                }
                #elif ( (GB_A_IS_SPARSE || GB_A_IS_HYPER) && GB_B_IS_FULL )
                {

                    //----------------------------------------------------------
                    // A is sparse/hyper and B is full
                    //----------------------------------------------------------

                    GB_GETC (cij, pC) ;             // cij = Cx [pC]
                    #if GB_IS_PAIR_MULTIPLIER
                    { 
                        #if GB_IS_ANY_MONOID
                        // ANY monoid: take the first entry found
                        GB_MULT (cij, ignore, ignore, 0, 0, 0) ;
                        #elif GB_IS_EQ_MONOID
                        // EQ_PAIR semiring
                        cij = (cij == 1) ;
                        #elif (GB_CTYPE_BITS > 0)
                        // PLUS, XOR monoids: A(:,i)'*B(:,j) is nnz(A(:,i)),
                        // for bool, 8-bit, 16-bit, or 32-bit integer
                        uint64_t t = ((uint64_t) cij) + ainz ;
                        cij = (GB_CTYPE) (t & GB_CTYPE_BITS) ;
                        #elif GB_IS_PLUS_FC32_MONOID
                        // PLUS monoid for float complex
                        cij = GxB_CMPLXF (crealf (cij) + (float) ainz, 0) ;
                        #elif GB_IS_PLUS_FC64_MONOID
                        // PLUS monoid for double complex
                        cij = GxB_CMPLX (creal (cij) + (double) ainz, 0) ;
                        #else
                        // PLUS monoid for float, double, or 64-bit integers 
                        cij += (GB_CTYPE) ainz ;
                        #endif
                    }
                    #else
                    {
                        GB_PRAGMA_SIMD_DOT (cij)
                        for (int64_t p = pA ; p < pA_end ; p++)
                        { 
                            GB_DOT_TERMINAL (cij) ;         // break if terminal
                            int64_t k = Ai [p] ;
                            // cij += A(k,i) * B(k,j)
                            GB_GETA (aki, Ax, p   ) ;       // aki = A(k,i)
                            GB_GETB (bkj, Bx, pB+k) ;       // bkj = B(k,j)
                            GB_MULTADD (cij, aki, bkj, i, k, j) ;
                        }
                    }
                    #endif
                    GB_DOT_ALWAYS_SAVE_CIJ ;

                }
                #elif ( (GB_A_IS_SPARSE || GB_A_IS_HYPER) && GB_B_IS_BITMAP )
                {

                    //----------------------------------------------------------
                    // A is sparse/hyper and B is bitmap
                    //----------------------------------------------------------

                    for (int64_t p = pA ; p < pA_end ; p++)
                    {
                        int64_t k = Ai [p] ;
                        if (Bb [pB+k])
                        { 
                            GB_DOT (k, p, pB+k) ;
                        }
                    }
                    GB_DOT_SAVE_CIJ ;

                }
                #else
                {

                    //----------------------------------------------------------
                    // both A and B are sparse/hyper
                    //----------------------------------------------------------

                    if (Ai [pA_end-1] < ib_first || ib_last < Ai [pA])
                    { 

                        //------------------------------------------------------
                        // pattern of A(:,i) and B(:,j) don't overlap
                        //------------------------------------------------------

                    }
                    else if (ainz > 8 * bjnz)
                    {

                        //------------------------------------------------------
                        // B(:,j) is very sparse compared to A(:,i)
                        //------------------------------------------------------

                        while (pA < pA_end && pB < pB_end)
                        {
                            int64_t ia = Ai [pA] ;
                            int64_t ib = Bi [pB] ;
                            if (ia < ib)
                            { 
                                // A(ia,i) appears before B(ib,j)
                                // discard all entries A(ia:ib-1,i)
                                int64_t pleft = pA + 1 ;
                                int64_t pright = pA_end - 1 ;
                                GB_TRIM_BINARY_SEARCH (ib, Ai, pleft, pright) ;
                                ASSERT (pleft > pA) ;
                                pA = pleft ;
                            }
                            else if (ib < ia)
                            { 
                                // B(ib,j) appears before A(ia,i)
                                pB++ ;
                            }
                            else // ia == ib == k
                            { 
                                // A(k,i) and B(k,j) are next entries to merge
                                GB_DOT (ia, pA, pB) ;
                                pA++ ;
                                pB++ ;
                            }
                        }
                        GB_DOT_SAVE_CIJ ;

                    }
                    else if (bjnz > 8 * ainz)
                    {

                        //------------------------------------------------------
                        // A(:,i) is very sparse compared to B(:,j)
                        //------------------------------------------------------

                        while (pA < pA_end && pB < pB_end)
                        {
                            int64_t ia = Ai [pA] ;
                            int64_t ib = Bi [pB] ;
                            if (ia < ib)
                            { 
                                // A(ia,i) appears before B(ib,j)
                                pA++ ;
                            }
                            else if (ib < ia)
                            { 
                                // B(ib,j) appears before A(ia,i)
                                // discard all entries B(ib:ia-1,j)
                                int64_t pleft = pB + 1 ;
                                int64_t pright = pB_end - 1 ;
                                GB_TRIM_BINARY_SEARCH (ia, Bi, pleft, pright) ;
                                ASSERT (pleft > pB) ;
                                pB = pleft ;
                            }
                            else // ia == ib == k
                            { 
                                // A(k,i) and B(k,j) are next entries to merge
                                GB_DOT (ia, pA, pB) ;
                                pA++ ;
                                pB++ ;
                            }
                        }
                        GB_DOT_SAVE_CIJ ;

                    }
                    else
                    {

                        //------------------------------------------------------
                        // A(:,i) and B(:,j) have about the same sparsity
                        //------------------------------------------------------

                        while (pA < pA_end && pB < pB_end)
                        {
                            int64_t ia = Ai [pA] ;
                            int64_t ib = Bi [pB] ;
                            if (ia < ib)
                            { 
                                // A(ia,i) appears before B(ib,j)
                                pA++ ;
                            }
                            else if (ib < ia)
                            { 
                                // B(ib,j) appears before A(ia,i)
                                pB++ ;
                            }
                            else // ia == ib == k
                            { 
                                // A(k,i) and B(k,j) are the entries to merge
                                GB_DOT (ia, pA, pB) ;
                                pA++ ;
                                pB++ ;
                            }
                        }
                        GB_DOT_SAVE_CIJ ;
                    }
                }
                #endif
            }
        }
    }
}

