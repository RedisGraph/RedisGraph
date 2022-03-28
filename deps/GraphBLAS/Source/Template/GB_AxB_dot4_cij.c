//------------------------------------------------------------------------------
// GB_AxB_dot4_cij.c: C(i,j) = A(:,i)'*B(:,j) for dot4 method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A is sparse or hypersparse, B is full or bitmap, and C is full

{

    //--------------------------------------------------------------------------
    // get C(i,j)
    //--------------------------------------------------------------------------

    const int64_t pC = i + pC_start ;   // C(i,j) is at Cx [pC]
    GB_CTYPE GB_GET4C (cij, pC) ;       // cij = Cx [pC]

    //--------------------------------------------------------------------------
    // C(i,j) += A (:,i)*B(:,j): a single dot product
    //--------------------------------------------------------------------------

    #if ( GB_B_IS_FULL )
    {

        //----------------------------------------------------------------------
        // A is sparse/hyper and B is full
        //----------------------------------------------------------------------

        #if GB_IS_PAIR_MULTIPLIER
        { 
            #if GB_IS_EQ_MONOID
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
        #elif GB_IS_MIN_FIRSTJ_SEMIRING
        {
            // MIN_FIRSTJ semiring: take the 1st entry in A(:,i)
            if (ainz > 0)
            { 
                int64_t k = Ai [pA] + GB_OFFSET ;
                cij = GB_IMIN (cij, k) ;
            }
        }
        #elif GB_IS_MAX_FIRSTJ_SEMIRING
        {
            // MAX_FIRSTJ semiring: take last entry in A(:,i)
            if (ainz > 0)
            { 
                int64_t k = Ai [pA_end-1] + GB_OFFSET ;
                cij = GB_IMAX (cij, k) ;
            }
        }
        #else
        {
            GB_PRAGMA_SIMD_DOT (cij)
            for (int64_t p = pA ; p < pA_end ; p++)
            { 
                int64_t k = Ai [p] ;
                GB_DOT (k, p, pB+k) ;   // cij += A(k,i)*B(k,j)
            }
        }
        #endif

    }
    #else
    {

        //----------------------------------------------------------------------
        // A is sparse/hyper and B is bitmap
        //----------------------------------------------------------------------

        #if GB_IS_MIN_FIRSTJ_SEMIRING
        {
            // MIN_FIRSTJ semiring: take the first entry
            for (int64_t p = pA ; p < pA_end ; p++)
            {
                int64_t k = Ai [p] ;
                if (Bb [pB+k])
                { 
                    cij = GB_IMIN (cij, k + GB_OFFSET) ;
                    break ;
                }
            }
        }
        #elif GB_IS_MAX_FIRSTJ_SEMIRING
        {
            // MAX_FIRSTJ semiring: take the last entry
            for (int64_t p = pA_end-1 ; p >= pA ; p--)
            {
                int64_t k = Ai [p] ;
                if (Bb [pB+k])
                { 
                    cij = GB_IMAX (cij, k + GB_OFFSET) ;
                    break ;
                }
            }
        }
        #else
        {
            GB_PRAGMA_SIMD_DOT (cij)
            for (int64_t p = pA ; p < pA_end ; p++)
            {
                int64_t k = Ai [p] ;
                if (Bb [pB+k])
                { 
                    GB_DOT (k, p, pB+k) ; // cij+=A(k,i)*B(k,j)
                }
            }

        }
        #endif

    }
    #endif

    //--------------------------------------------------------------------------
    // save C(i,j)
    //--------------------------------------------------------------------------

    Cx [pC] = cij ;
}

