//------------------------------------------------------------------------------
// GB_AxB_dot_cij.h: definitions for GB_AxB_dot*_cij.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_DOT: cij += A(k,i) * B(k,j), then break if terminal
// Ai [pA] and Bi [pB] are both equal to the index k.

// use the boolean flag cij_exists to set/check if C(i,j) exists
#undef  GB_CIJ_CHECK
#define GB_CIJ_CHECK true
#undef  GB_CIJ_EXISTS
#define GB_CIJ_EXISTS cij_exists
#undef  GB_DOT

#if GB_IS_PLUS_PAIR_REAL_SEMIRING

    //--------------------------------------------------------------------------
    // plus_pair_real semiring
    //--------------------------------------------------------------------------

    #if GB_CTYPE_IGNORE_OVERFLOW

        // PLUS_PAIR for 64-bit integers, float, and double (not complex):
        // To check if C(i,j) exists, test (cij != 0) when done.  The
        // boolean flag cij_exists is not used.
        #undef  GB_CIJ_CHECK
        #define GB_CIJ_CHECK false
        #undef  GB_CIJ_EXISTS
        #define GB_CIJ_EXISTS (cij != 0)
        #define GB_DOT(k,pA,pB) cij++ ;

    #else

        // PLUS_PAIR semiring for small integers
        #define GB_DOT(k,pA,pB)                                         \
        {                                                               \
            cij_exists = true ;                                         \
            cij++ ;                                                     \
        }

    #endif

#elif GB_IS_ANY_MONOID

    //--------------------------------------------------------------------------
    // ANY monoid
    //--------------------------------------------------------------------------

    #if defined ( GB_DOT3 )

        // for the dot3 method: C is sparse or hyper
        #define GB_DOT(k,pA,pB)                                         \
        {                                                               \
            GB_GETA (aki, Ax, pA) ;  /* aki = A(k,i) */                 \
            GB_GETB (bkj, Bx, pB) ;  /* bkj = B(k,j) */                 \
            /* cij = (A')(i,k) * B(k,j), and add to the pattern */      \
            cij_exists = true ;                                         \
            GB_MULT (cij, aki, bkj, i, k, j) ;                          \
            break ;                                                     \
        }

    #else

        // for the dot2 method: C is bitmap
        #define GB_DOT(k,pA,pB)                                         \
        {                                                               \
            GB_GETA (aki, Ax, pA) ;  /* aki = A(k,i) */                 \
            GB_GETB (bkj, Bx, pB) ;  /* bkj = B(k,j) */                 \
            /* cij = (A')(i,k) * B(k,j), and add to the pattern */      \
            GB_MULT (cij, aki, bkj, i, k, j) ;                          \
            int64_t pC = pC_start + i ;                                 \
            GB_PUTC (cij, pC) ;                                         \
            Cb [pC] = 1 ;                                               \
            task_cnvals++ ;                                             \
            break ;                                                     \
        }

    #endif

#else

    //--------------------------------------------------------------------------
    // all other semirings
    //--------------------------------------------------------------------------

    #define GB_DOT(k,pA,pB)                                             \
    {                                                                   \
        GB_GETA (aki, Ax, pA) ;  /* aki = A(k,i) */                     \
        GB_GETB (bkj, Bx, pB) ;  /* bkj = B(k,j) */                     \
        if (cij_exists)                                                 \
        {                                                               \
            /* cij += (A')(i,k) * B(k,j) */                             \
            GB_MULTADD (cij, aki, bkj, i, k, j) ;                       \
        }                                                               \
        else                                                            \
        {                                                               \
            /* cij = (A')(i,k) * B(k,j), and add to the pattern */      \
            cij_exists = true ;                                         \
            GB_MULT (cij, aki, bkj, i, k, j) ;                          \
        }                                                               \
        /* if (cij is terminal) break ; */                              \
        GB_DOT_TERMINAL (cij) ;                                         \
    }

#endif

