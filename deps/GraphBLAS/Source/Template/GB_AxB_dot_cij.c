//------------------------------------------------------------------------------
// GB_AxB_dot_cij: compute C(i,j) = A(:,i)'*B(:,j)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// computes C(i,j) = A (:,i)'*B(:,j) via sparse dot product.  This template is
// used for all three cases: C=A'*B and C<!M>=A'*B in dot2, and C<M>=A'*B in
// dot3.

// GB_AxB_dot2 defines either one of these, and uses this template twice:

//      GB_PHASE_1_OF_2 ; determine if cij exists, and increment C_count
//      GB_PHASE_2_OF_2 : 2nd phase, compute cij, no realloc of C

// GB_AxB_dot3 defines GB_DOT3, and uses this template just once.

// Only one of the three are #defined: either GB_PHASE_1_OF_2, GB_PHASE_2_OF_2,
// or GB_DOT3.

// When used as the multiplicative operator, the PAIR operator provides some
// useful special cases.  Its output is always one, for any matching pair of
// entries A(k,i)'*B(k,j) for some k.  If the monoid is ANY, then C(i,j)=1 if
// the intersection for the dot product is non-empty.  This intersection has to
// be found, in general.  However, suppose B(:,j) is dense.  Then every entry
// in the pattern of A(:,i)' will produce a 1 from the PAIR operator.  If the
// monoid is ANY, then C(i,j)=1 if A(:,i)' is nonempty.  If the monoid is PLUS,
// then C(i,j) is simply nnz(A(:,i)), assuming no overflow.  The XOR monoid
// acts like a 1-bit summation, so the result of the XOR_PAIR_BOOL semiring
// will be C(i,j) = mod (nnz(A(:,j)),2).

// If both A(:,i) and B(:,j) are sparse, then the intersection must still be
// found, so these optimizations can be used only if A(:,i) and/or B(:,j) are
// fully populated.

// For built-in, pre-generated semirings, the PAIR operator is only coupled
// with either the ANY, PLUS, EQ, or XOR monoids, since the other monoids are
// equivalent to the ANY monoid.  With no accumulator, EQ is the same as ANY,
// they differ for the C+=A'*B operation (see *dot4*).

#include "GB_unused.h"

// cij += A(k,i) * B(k,j), for merge operation
#undef  GB_DOT_MERGE
#define GB_DOT_MERGE                                                \
{                                                                   \
    GB_GETA (aki, Ax, pA) ;             /* aki = A(k,i) */          \
    GB_GETB (bkj, Bx, pB) ;             /* bkj = B(k,j) */          \
    if (cij_exists)                                                 \
    {                                                               \
        GB_MULTADD (cij, aki, bkj) ;    /* cij += aki * bkj */      \
    }                                                               \
    else                                                            \
    {                                                               \
        /* cij = A(k,i) * B(k,j), and add to the pattern */         \
        cij_exists = true ;                                         \
        GB_MULT (cij, aki, bkj) ;       /* cij = aki * bkj */       \
    }                                                               \
}

{

    //--------------------------------------------------------------------------
    // get the start of A(:,i) and B(:,j)
    //--------------------------------------------------------------------------

    bool cij_exists = false ;   // C(i,j) not yet in the pattern
    int64_t pB = pB_start ;
    int64_t ainz = pA_end - pA ;
    ASSERT (ainz >= 0) ;

    //--------------------------------------------------------------------------
    // declare the cij scalar
    //--------------------------------------------------------------------------

    #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DOT3 )
    GB_CIJ_DECLARE (cij) ;
    #endif

    //--------------------------------------------------------------------------
    // compute C(i,j) = A(:,i)' * B(j,:)
    //--------------------------------------------------------------------------

    if (ainz == 0)
    { 

        //----------------------------------------------------------------------
        // A(:,i) is empty so C(i,j) cannot be present
        //----------------------------------------------------------------------

        ;

    }
    else if (Ai [pA_end-1] < ib_first || ib_last < Ai [pA])
    { 

        //----------------------------------------------------------------------
        // pattern of A(:,i) and B(:,j) do not overlap
        //----------------------------------------------------------------------

        ;

    }
    else if (bjnz == bvlen && ainz == bvlen)
    {

        //----------------------------------------------------------------------
        // both A(:,i) and B(:,j) are dense
        //----------------------------------------------------------------------

        cij_exists = true ;

        #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DOT3 )
            #if GB_IS_PAIR_MULTIPLIER

                #if (GB_IS_ANY_MONOID || GB_IS_EQ_MONOID)
                // ANY monoid: take the first entry found
                cij = 1 ;
                #elif (GB_CTYPE_BITS > 0)
                // PLUS, XOR monoids: A(:,i)'*B(:,j) is nnz(A(:,i)),
                // for bool, 8-bit, 16-bit, or 32-bit integer
                cij = (GB_CTYPE) (((uint64_t) bvlen) & GB_CTYPE_BITS) ;
                #else
                // PLUS monoid for float, double, or 64-bit integers 
                cij = (GB_CTYPE) bvlen ;
                #endif

            #else

                // cij = A(0,i) * B(0,j)
                GB_GETA (aki, Ax, pA) ;             // aki = A(0,i)
                GB_GETB (bkj, Bx, pB) ;             // bkj = B(0,j)
                GB_MULT (cij, aki, bkj) ;           // cij = aki * bkj
                GB_PRAGMA_VECTORIZE_DOT
                for (int64_t k = 1 ; k < bvlen ; k++)
                { 
                    GB_DOT_TERMINAL (cij) ;             // break if cij terminal
                    // cij += A(k,i) * B(k,j)
                    GB_GETA (aki, Ax, pA+k) ;           // aki = A(k,i)
                    GB_GETB (bkj, Bx, pB+k) ;           // bkj = B(k,j)
                    GB_MULTADD (cij, aki, bkj) ;        // cij += aki * bkj
                }

            #endif
        #endif


    }
    else if (ainz == bvlen)
    {

        //----------------------------------------------------------------------
        // A(:,i) is dense and B(:,j) is sparse
        //----------------------------------------------------------------------

        cij_exists = true ;

        #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DOT3 )
            #if GB_IS_PAIR_MULTIPLIER

                #if (GB_IS_ANY_MONOID || GB_IS_EQ_MONOID)
                // ANY monoid: take the first entry found
                cij = 1 ;
                #elif (GB_CTYPE_BITS > 0)
                // PLUS, XOR monoids: A(:,i)'*B(:,j) is nnz(A(:,i)),
                // for bool, 8-bit, 16-bit, or 32-bit integer
                cij = (GB_CTYPE) (((uint64_t) bjnz) & GB_CTYPE_BITS) ;
                #else
                // PLUS monoid for float, double, or 64-bit integers 
                cij = (GB_CTYPE) bjnz ;
                #endif

            #else

                int64_t k = Bi [pB] ;               // first row index of B(:,j)
                // cij = A(k,i) * B(k,j)
                GB_GETA (aki, Ax, pA+k) ;           // aki = A(k,i)
                GB_GETB (bkj, Bx, pB  ) ;           // bkj = B(k,j)
                GB_MULT (cij, aki, bkj) ;           // cij = aki * bkj
                GB_PRAGMA_VECTORIZE_DOT
                for (int64_t p = pB+1 ; p < pB_end ; p++)
                { 
                    GB_DOT_TERMINAL (cij) ;             // break if cij terminal
                    int64_t k = Bi [p] ;                // next index of B(:,j)
                    // cij += A(k,i) * B(k,j)
                    GB_GETA (aki, Ax, pA+k) ;           // aki = A(k,i)
                    GB_GETB (bkj, Bx, p   ) ;           // bkj = B(k,j)
                    GB_MULTADD (cij, aki, bkj) ;        // cij += aki * bkj
                }

            #endif
        #endif

    }
    else if (bjnz == bvlen)
    {

        //----------------------------------------------------------------------
        // A(:,i) is sparse and B(:,j) is dense
        //----------------------------------------------------------------------

        cij_exists = true ;

        #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DOT3 )
            #if GB_IS_PAIR_MULTIPLIER

                #if (GB_IS_ANY_MONOID || GB_IS_EQ_MONOID)
                // ANY monoid: take the first entry found
                cij = 1 ;
                #elif (GB_CTYPE_BITS > 0)
                // PLUS, XOR monoids: A(:,i)'*B(:,j) is nnz(A(:,i)),
                // for bool, 8-bit, 16-bit, or 32-bit integer
                cij = (GB_CTYPE) (((uint64_t) ainz) & GB_CTYPE_BITS) ;
                #else
                // PLUS monoid for float, double, or 64-bit integers 
                cij = (GB_CTYPE) ainz ;
                #endif

            #else

                int64_t k = Ai [pA] ;               // first row index of A(:,i)
                // cij = A(k,i) * B(k,j)
                GB_GETA (aki, Ax, pA  ) ;           // aki = A(k,i)
                GB_GETB (bkj, Bx, pB+k) ;           // bkj = B(k,j)
                GB_MULT (cij, aki, bkj) ;           // cij = aki * bkj
                GB_PRAGMA_VECTORIZE_DOT
                for (int64_t p = pA+1 ; p < pA_end ; p++)
                { 
                    GB_DOT_TERMINAL (cij) ;             // break if cij terminal
                    int64_t k = Ai [p] ;                // next index of A(:,i)
                    // cij += A(k,i) * B(k,j)
                    GB_GETA (aki, Ax, p   ) ;           // aki = A(k,i)
                    GB_GETB (bkj, Bx, pB+k) ;           // bkj = B(k,j)
                    GB_MULTADD (cij, aki, bkj) ;        // cij += aki * bkj
                }

            #endif
        #endif

    }
    else if (ainz > 8 * bjnz)
    {

        //----------------------------------------------------------------------
        // B(:,j) is very sparse compared to A(:,i)
        //----------------------------------------------------------------------

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
                // A(k,i) and B(k,j) are the next entries to merge
                #if defined ( GB_PHASE_1_OF_2 )
                cij_exists = true ;
                break ;
                #else
                GB_DOT_MERGE
                GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                pA++ ;
                pB++ ;
                #endif
            }
        }

    }
    else if (bjnz > 8 * ainz)
    {

        //----------------------------------------------------------------------
        // A(:,i) is very sparse compared to B(:,j)
        //----------------------------------------------------------------------

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
                // A(k,i) and B(k,j) are the next entries to merge
                #if defined ( GB_PHASE_1_OF_2 )
                cij_exists = true ;
                break ;
                #else
                GB_DOT_MERGE
                GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                pA++ ;
                pB++ ;
                #endif
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // A(:,i) and B(:,j) have about the same sparsity
        //----------------------------------------------------------------------

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
                // A(k,i) and B(k,j) are the next entries to merge
                #if defined ( GB_PHASE_1_OF_2 )
                cij_exists = true ;
                break ;
                #else
                GB_DOT_MERGE
                GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                pA++ ;
                pB++ ;
                #endif
            }
        }
    }

    //--------------------------------------------------------------------------
    // save C(i,j)
    //--------------------------------------------------------------------------

    #if defined ( GB_DOT3 )

        // GB_AxB_dot3: computing C<M>=A'*B
        if (cij_exists)
        { 
            // C(i,j) = cij
            GB_CIJ_SAVE (cij, pC) ;
            Ci [pC] = i ;
        }
        else
        { 
            // C(i,j) becomes a zombie
            task_nzombies++ ;
            Ci [pC] = GB_FLIP (i) ;
        }

    #else

        // GB_AxB_dot2: computing C=A'*B or C<!M>=A'*B
        if (cij_exists)
        { 
            // C(i,j) = cij
            #if defined ( GB_PHASE_1_OF_2 )
                C_count [Iter_k] ++ ;
            #else
                GB_CIJ_SAVE (cij, cnz) ;
                Ci [cnz++] = i ;
                if (cnz > cnz_last) break ;
            #endif
        }

    #endif
}

