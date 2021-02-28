//------------------------------------------------------------------------------
// GB_AxB_dot4:  C+=A'*B via dot products, where C is dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C+=A'*B where C is a dense matrix and computed in-place.  The monoid of the
// semiring matches the accum operator, and the type of C matches the ztype of
// accum.  That is, no typecasting can be done with C.

// The PAIR operator as the multiplier provides important special cases.
// See Template/GB_AxB_dot_cij.c for details.

// cij += A(k,i) * B(k,j)
#undef  GB_DOT_MERGE
#define GB_DOT_MERGE                                                \
{                                                                   \
    if (!cij_updated)                                               \
    {                                                               \
        cij_updated = true ;                                        \
        GB_GETC (cij, pC) ;                                         \
    }                                                               \
    GB_GETA (aki, Ax, pA) ;         /* aki = A(k,i) */              \
    GB_GETB (bkj, Bx, pB) ;         /* bkj = B(k,j) */              \
    GB_MULTADD (cij, aki, bkj) ;    /* cij += aki * bkj */          \
    GB_DOT_TERMINAL (cij) ;         /* break if cij == terminal */  \
    pA++ ;                                                          \
    pB++ ;                                                          \
}

{

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    GB_CTYPE *GB_RESTRICT Cx = C->x ;
    const int64_t cvlen = C->vlen ;

    const int64_t  *GB_RESTRICT Bp = B->p ;
    const int64_t  *GB_RESTRICT Bh = B->h ;
    const int64_t  *GB_RESTRICT Bi = B->i ;
    const GB_BTYPE *GB_RESTRICT Bx = B_is_pattern ? NULL : B->x ;
    const int64_t bvlen = B->vlen ;

    const int64_t  *GB_RESTRICT Ap = A->p ;
    const int64_t  *GB_RESTRICT Ah = A->h ;
    const int64_t  *GB_RESTRICT Ai = A->i ;
    const GB_ATYPE *GB_RESTRICT Ax = A_is_pattern ? NULL : A->x ;
    ASSERT (A->vlen == B->vlen) ;

    int ntasks = naslice * nbslice ;

    //--------------------------------------------------------------------------
    // C += A'*B
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {

        //----------------------------------------------------------------------
        // get the entries in A and B to compute
        //----------------------------------------------------------------------

        int a_taskid = taskid / nbslice ;
        int b_taskid = taskid % nbslice ;

        int64_t akfirst = A_slice [a_taskid] ;
        int64_t aklast  = A_slice [a_taskid+1] ;
        if (akfirst >= aklast) continue ;

        int64_t bkfirst = B_slice [b_taskid] ;
        int64_t bklast  = B_slice [b_taskid+1] ;
        if (bkfirst >= bklast) continue ;

        //----------------------------------------------------------------------
        // C+=A'*B via dot products
        //----------------------------------------------------------------------

        for (int64_t bk = bkfirst ; bk < bklast ; bk++)
        {

            //------------------------------------------------------------------
            // get B(:,j)
            //------------------------------------------------------------------

            int64_t j = (Bh == NULL) ? bk : Bh [bk] ;
            int64_t pB_start = Bp [bk] ;
            int64_t pB_end   = Bp [bk+1] ;
            int64_t pC_start = j * cvlen ;
            int64_t bjnz = pB_end - pB_start ;
            if (bjnz == 0) continue ;

            if (bjnz == bvlen)
            {

                //--------------------------------------------------------------
                // B(:,j) is dense
                //--------------------------------------------------------------

                for (int64_t ak = akfirst ; ak < aklast ; ak++)
                {

                    //----------------------------------------------------------
                    // get A(:,i)
                    //----------------------------------------------------------

                    int64_t i = (Ah == NULL) ? ak : Ah [ak] ;
                    int64_t pA     = Ap [ak] ;
                    int64_t pA_end = Ap [ak+1] ;
                    int64_t ainz = pA_end - pA ;
                    if (ainz == 0) continue ;

                    GB_CIJ_DECLARE (cij) ;          // declare the cij scalar
                    int64_t pC = i + pC_start ;     // C(i,j) is at Cx [pC]
                    int64_t pB = pB_start ;
                    GB_GETC (cij, pC) ;             // cij = Cx [pC]

                    //----------------------------------------------------------
                    // special cases for the PAIR multiplier
                    //----------------------------------------------------------

                    // Since B(:,j) is dense, C(i,j) += A(:,i)'*B(:,j) is
                    // trivial to compute with the PAIR multiplier.

                    #if GB_IS_PAIR_MULTIPLIER

                        #if GB_IS_ANY_MONOID
                        // ANY monoid: take the first entry found
                        cij = 1 ;
                        #elif GB_IS_EQ_MONOID
                        // A(:,i)'*B(:j) is one, so this result must be
                        // accumulated into cij, as cij += 1, where the
                        // accumulator is the EQ operator.
                        cij = (cij == 1) ;
                        #elif (GB_CTYPE_BITS > 0)
                        // PLUS, XOR monoids: A(:,i)'*B(:,j) is nnz(A(:,i)),
                        // for bool, 8-bit, 16-bit, or 32-bit integer
                        uint64_t t = ((uint64_t) cij) + ainz ;
                        cij = (GB_CTYPE) (t & GB_CTYPE_BITS) ;
                        #else
                        // PLUS monoid for float, double, or 64-bit integers 
                        cij += (GB_CTYPE) ainz ;
                        #endif

                    #else

                    //----------------------------------------------------------
                    // general case
                    //----------------------------------------------------------

                    if (ainz == bvlen)
                    {

                        //------------------------------------------------------
                        // both A(:,i) and B(:,j) are dense
                        //------------------------------------------------------

                        GB_PRAGMA_VECTORIZE_DOT
                        for (int64_t k = 0 ; k < bvlen ; k++)
                        { 
                            GB_DOT_TERMINAL (cij) ;         // break if terminal
                            // cij += A(k,i) * B(k,j)
                            GB_GETA (aki, Ax, pA+k) ;       // aki = A(k,i)
                            GB_GETB (bkj, Bx, pB+k) ;       // bkj = B(k,j)
                            GB_MULTADD (cij, aki, bkj) ;    // cij += aki * bkj
                        }

                    }
                    else
                    {

                        //------------------------------------------------------
                        // A(:,i) is sparse and B(:,j) is dense
                        //------------------------------------------------------

                        GB_PRAGMA_VECTORIZE_DOT
                        for (int64_t p = pA ; p < pA_end ; p++)
                        { 
                            GB_DOT_TERMINAL (cij) ;         // break if terminal
                            int64_t k = Ai [p] ;
                            // cij += A(k,i) * B(k,j)
                            GB_GETA (aki, Ax, p   ) ;       // aki = A(k,i)
                            GB_GETB (bkj, Bx, pB+k) ;       // bkj = B(k,j)
                            GB_MULTADD (cij, aki, bkj) ;    // cij += aki * bkj
                        }
                    }

                    #endif
                    GB_PUTC (cij, pC) ;                 // Cx [pC] = cij
                }

            }
            else
            {

                //--------------------------------------------------------------
                // B(:,j) is sparse
                //--------------------------------------------------------------

                // get the first and last index in B(:,j)
                int64_t ib_first = Bi [pB_start] ;
                int64_t ib_last  = Bi [pB_end-1] ;

                for (int64_t ak = akfirst ; ak < aklast ; ak++)
                {

                    //----------------------------------------------------------
                    // get A(:,i)
                    //----------------------------------------------------------

                    int64_t i = (Ah == NULL) ? ak : Ah [ak] ;
                    int64_t pA     = Ap [ak] ;
                    int64_t pA_end = Ap [ak+1] ;
                    int64_t ainz = pA_end - pA ;
                    if (ainz == 0) continue ;
                    // get the first and last index in A(:,i)
                    if (Ai [pA_end-1] < ib_first || ib_last < Ai [pA]) continue;

                    //----------------------------------------------------------
                    // C(i,j) += A(:,i)'*B(:,j)
                    //----------------------------------------------------------

                    GB_CIJ_DECLARE (cij) ;          // declare the cij scalar
                    int64_t pC = i + pC_start ;     // C(i,j) is at Cx [pC]
                    int64_t pB = pB_start ;

                    if (ainz == bvlen)
                    {

                        //------------------------------------------------------
                        // A(:,i) is dense and B(:,j) is sparse
                        //------------------------------------------------------

                        GB_GETC (cij, pC) ;                 // cij = Cx [pC]

                        #if GB_IS_PAIR_MULTIPLIER

                            #if GB_IS_ANY_MONOID
                            // ANY monoid: take the first entry found
                            cij = 1 ;
                            #elif GB_IS_EQ_MONOID
                            // A(:,i)'*B(:j) is one, so this result must be
                            // accumulated into cij, as cij += 1, where the
                            // accumulator is the EQ operator.
                            cij = (cij == 1) ;
                            #elif (GB_CTYPE_BITS > 0)
                            // PLUS, XOR monoids: A(:,i)'*B(:,j) is nnz(A(:,i)),
                            // for bool, 8-bit, 16-bit, or 32-bit integer
                            uint64_t t = ((uint64_t) cij) + bjnz ;
                            cij = (GB_CTYPE) (t & GB_CTYPE_BITS) ;
                            #else
                            // PLUS monoid for float, double, or 64-bit integers
                            cij += (GB_CTYPE) bjnz ;
                            #endif

                        #else

                            GB_PRAGMA_VECTORIZE_DOT
                            for (int64_t p = pB ; p < pB_end ; p++)
                            { 
                                GB_DOT_TERMINAL (cij) ;   // break if terminal
                                int64_t k = Bi [p] ;
                                // cij += A(k,i) * B(k,j)
                                GB_GETA (aki, Ax, pA+k) ;     // aki = A(k,i)
                                GB_GETB (bkj, Bx, p   ) ;     // bkj = B(k,j)
                                GB_MULTADD (cij, aki, bkj) ;  // cij += aki*bkj
                            }

                        #endif

                        GB_PUTC (cij, pC) ;                 // Cx [pC] = cij

                    }
                    else if (ainz > 8 * bjnz)
                    {

                        //------------------------------------------------------
                        // B(:,j) is very sparse compared to A(:,i)
                        //------------------------------------------------------

                        bool cij_updated = false ;
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
                                GB_DOT_MERGE ;
                            }
                        }
                        if (cij_updated) GB_PUTC (cij, pC) ;

                    }
                    else if (bjnz > 8 * ainz)
                    {

                        //------------------------------------------------------
                        // A(:,i) is very sparse compared to B(:,j)
                        //------------------------------------------------------

                        bool cij_updated = false ;
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
                                GB_DOT_MERGE ;
                            }
                        }
                        if (cij_updated) GB_PUTC (cij, pC) ;

                    }
                    else
                    {

                        //------------------------------------------------------
                        // A(:,i) and B(:,j) have about the same sparsity
                        //------------------------------------------------------

                        bool cij_updated = false ;
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
                                GB_DOT_MERGE ;
                            }
                        }
                        if (cij_updated) GB_PUTC (cij, pC) ;
                    }
                }
            }
        }
    }
}

