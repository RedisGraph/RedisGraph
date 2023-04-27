//------------------------------------------------------------------------------
// GB_AxB_saxpy3_template.h: C=A*B, C<M>=A*B, or C<!M>=A*B via saxpy3 method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Definitions for GB_AxB_saxpy3_template.c.  These do not depend on the
// sparsity of A and B.

#ifndef GB_AXB_SAXPY3_TEMPLATE_H
#define GB_AXB_SAXPY3_TEMPLATE_H

//------------------------------------------------------------------------------
// GB_GET_M_j: prepare to iterate over M(:,j)
//------------------------------------------------------------------------------

// prepare to iterate over the vector M(:,j), for the (kk)th vector of B
// FUTURE::: lookup all M(:,j) for all vectors in B, in a single pass,
// and save the mapping (like C_to_M mapping in GB_ewise_slice)
#define GB_GET_M_j                                              \
    int64_t mpleft = 0 ;                                        \
    int64_t mpright = mnvec-1 ;                                 \
    int64_t pM_start, pM_end ;                                  \
    GB_lookup (M_is_hyper, Mh, Mp, mvlen, &mpleft, mpright,     \
        GBH (Bh, kk), &pM_start, &pM_end) ;                     \
    const int64_t mjnz = pM_end - pM_start ;

//------------------------------------------------------------------------------
// GB_GET_M_j_RANGE
//------------------------------------------------------------------------------

#define GB_GET_M_j_RANGE(gamma)                                 \
    const int64_t mjnz_much = mjnz * gamma

//------------------------------------------------------------------------------
// GB_SCATTER_Mj_t: scatter M(:,j) of the given type into Gus. workspace
//------------------------------------------------------------------------------

#define GB_SCATTER_Mj_t(mask_t,pMstart,pMend,mark)                      \
{                                                                       \
    const mask_t *restrict Mxx = (mask_t *) Mx ;                        \
    if (M_is_bitmap)                                                    \
    {                                                                   \
        /* M is bitmap */                                               \
        for (int64_t pM = pMstart ; pM < pMend ; pM++)                  \
        {                                                               \
            /* if (M (i,j) == 1) mark Hf [i] */                         \
            if (Mb [pM] && Mxx [pM]) Hf [GBI (Mi, pM, mvlen)] = mark ;  \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        /* M is hyper, sparse, or full */                               \
        for (int64_t pM = pMstart ; pM < pMend ; pM++)                  \
        {                                                               \
            /* if (M (i,j) == 1) mark Hf [i] */                         \
            if (Mxx [pM]) Hf [GBI (Mi, pM, mvlen)] = mark ;             \
        }                                                               \
    }                                                                   \
}                                                                       \
break ;

//------------------------------------------------------------------------------
// GB_SCATTER_M_j:  scatter M(:,j) into the Gustavson workpace
//------------------------------------------------------------------------------

#define GB_SCATTER_M_j(pMstart,pMend,mark)                                  \
    if (Mx == NULL)                                                         \
    {                                                                       \
        /* M is structural, not valued */                                   \
        if (M_is_bitmap)                                                    \
        {                                                                   \
            /* M is bitmap */                                               \
            for (int64_t pM = pMstart ; pM < pMend ; pM++)                  \
            {                                                               \
                /* if (M (i,j) is present) mark Hf [i] */                   \
                if (Mb [pM]) Hf [GBI (Mi, pM, mvlen)] = mark ;              \
            }                                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* M is hyper, sparse, or full */                               \
            for (int64_t pM = pMstart ; pM < pMend ; pM++)                  \
            {                                                               \
                /* mark Hf [i] */                                           \
                Hf [GBI (Mi, pM, mvlen)] = mark ;                           \
            }                                                               \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* mask is valued, not structural */                                \
        switch (msize)                                                      \
        {                                                                   \
            default:                                                        \
            case GB_1BYTE: GB_SCATTER_Mj_t (uint8_t , pMstart, pMend, mark) ; \
            case GB_2BYTE: GB_SCATTER_Mj_t (uint16_t, pMstart, pMend, mark) ; \
            case GB_4BYTE: GB_SCATTER_Mj_t (uint32_t, pMstart, pMend, mark) ; \
            case GB_8BYTE: GB_SCATTER_Mj_t (uint64_t, pMstart, pMend, mark) ; \
            case GB_16BYTE:                                                 \
            {                                                               \
                const uint64_t *restrict Mxx = (uint64_t *) Mx ;            \
                for (int64_t pM = pMstart ; pM < pMend ; pM++)              \
                {                                                           \
                    /* if (M (i,j) == 1) mark Hf [i] */                     \
                    if (!GBB (Mb, pM)) continue ;                           \
                    if (Mxx [2*pM] || Mxx [2*pM+1])                         \
                    {                                                       \
                        /* Hf [i] = M(i,j) */                               \
                        Hf [GBI (Mi, pM, mvlen)] = mark ;                   \
                    }                                                       \
                }                                                           \
            }                                                               \
        }                                                                   \
    }

//------------------------------------------------------------------------------
// GB_HASH_M_j: scatter M(:,j) for a coarse hash task
//------------------------------------------------------------------------------

// hash M(:,j) into Hf and Hi for coarse hash task, C<M>=A*B or C<!M>=A*B
#define GB_HASH_M_j                                         \
    for (int64_t pM = pM_start ; pM < pM_end ; pM++)        \
    {                                                       \
        GB_GET_M_ij (pM) ;      /* get M(i,j) */            \
        if (!mij) continue ;    /* skip if M(i,j)=0 */      \
        const int64_t i = GBI (Mi, pM, mvlen) ;             \
        for (GB_HASH (i))       /* find i in hash */        \
        {                                                   \
            if (Hf [hash] < mark)                           \
            {                                               \
                Hf [hash] = mark ;  /* insert M(i,j)=1 */   \
                Hi [hash] = i ;                             \
                break ;                                     \
            }                                               \
        }                                                   \
    }

//------------------------------------------------------------------------------
// GB_GET_T_FOR_SECONDJ: define t for SECONDJ and SECONDJ1 semirings
//------------------------------------------------------------------------------

#if GB_IS_SECONDJ_MULTIPLIER
    #define GB_GET_T_FOR_SECONDJ                            \
        GB_CIJ_DECLARE (t) ;                                \
        GB_MULT (t, ignore, ignore, ignore, ignore, j) ;
#else
    #define GB_GET_T_FOR_SECONDJ
#endif

//------------------------------------------------------------------------------
// GB_GET_B_j_FOR_ALL_FORMATS: prepare to iterate over B(:,j)
//------------------------------------------------------------------------------

// prepare to iterate over the vector B(:,j), the (kk)th vector in B, where 
// j == GBH (Bh, kk).  This macro works regardless of the sparsity of A and B.
#define GB_GET_B_j_FOR_ALL_FORMATS(A_is_hyper,B_is_sparse,B_is_hyper)       \
    int64_t pleft = 0 ;                                                     \
    int64_t pright = anvec-1 ;                                              \
    const int64_t j = (B_is_hyper) ? Bh [kk] : kk ;                         \
    GB_GET_T_FOR_SECONDJ ;  /* t = j for SECONDJ, or j+1 for SECONDJ1 */    \
    int64_t pB = (B_is_sparse || B_is_hyper) ? Bp [kk] : (kk * bvlen) ;     \
    const int64_t pB_end =                                                  \
        (B_is_sparse || B_is_hyper) ? Bp [kk+1] : (pB+bvlen) ;              \
    const int64_t bjnz = pB_end - pB ;  /* nnz (B (:,j) */                  \
    /* FUTURE::: can skip if mjnz == 0 for C<M>=A*B tasks */                \
    if (A_is_hyper && (B_is_sparse || B_is_hyper) && bjnz > 2 && !B_jumbled)\
    {                                                                       \
        /* trim Ah [0..pright] to remove any entries past last B(:,j), */   \
        /* to speed up GB_lookup in GB_GET_A_k_FOR_ALL_FORMATS. */          \
        /* This requires that B is not jumbled */                           \
        GB_bracket_right (GBI (Bi, pB_end-1, bvlen), Ah, 0, &pright) ;      \
    }

//------------------------------------------------------------------------------
// GB_GET_B_kj: get the numeric value of B(k,j)
//------------------------------------------------------------------------------

#if GB_IS_FIRSTJ_MULTIPLIER

    // FIRSTJ or FIRSTJ1 multiplier
    // t = aik * bkj = k or k+1
    #define GB_GET_B_kj                                     \
        GB_CIJ_DECLARE (t) ;                                \
        GB_MULT (t, ignore, ignore, ignore, k, ignore)

#else

    #define GB_GET_B_kj \
        GB_GETB (bkj, Bx, pB, B_iso)       /* bkj = Bx [pB] */

#endif

//------------------------------------------------------------------------------
// GB_GET_A_k_FOR_ALL_FORMATS: prepare to iterate over the vector A(:,k)
//------------------------------------------------------------------------------

#define GB_GET_A_k_FOR_ALL_FORMATS(A_is_hyper)                              \
    if (B_jumbled) pleft = 0 ;  /* reuse pleft if B is not jumbled */       \
    int64_t pA_start, pA_end ;                                              \
    GB_lookup (A_is_hyper, Ah, Ap, avlen, &pleft, pright, k,                \
        &pA_start, &pA_end) ;                                               \
    const int64_t aknz = pA_end - pA_start

//------------------------------------------------------------------------------
// GB_GET_M_ij: get the numeric value of M(i,j)
//------------------------------------------------------------------------------

#define GB_GET_M_ij(pM)                             \
    /* get M(i,j), at Mi [pM] and Mx [pM] */        \
    bool mij = GBB (Mb, pM) && GB_mcast (Mx, pM, msize)

//------------------------------------------------------------------------------
// GB_MULT_A_ik_B_kj: declare t and compute t = A(i,k) * B(k,j)
//------------------------------------------------------------------------------

#if GB_IS_PAIR_MULTIPLIER

    // PAIR multiplier: t is always 1; no numeric work to do to compute t.
    // The LXOR_PAIR and PLUS_PAIR semirings need the value t = 1 to use in
    // their monoid operator, however.
    #define t (GB_CTYPE_CAST (1, 0))
    #define GB_MULT_A_ik_B_kj

#elif ( GB_IS_FIRSTJ_MULTIPLIER || GB_IS_SECONDJ_MULTIPLIER )

    // nothing to do; t = aik*bkj already defined in an outer loop
    #define GB_MULT_A_ik_B_kj

#else

    // typical semiring
    #define GB_MULT_A_ik_B_kj                                       \
        GB_GETA (aik, Ax, pA, A_iso) ;  /* aik = Ax [pA] ;  */      \
        GB_CIJ_DECLARE (t) ;            /* ctype t ;        */      \
        GB_MULT (t, aik, bkj, i, k, j)  /* t = aik * bkj ;  */

#endif

//------------------------------------------------------------------------------
// GB_GATHER_ALL_C_j: gather the values and pattern of C(:,j)
//------------------------------------------------------------------------------

// gather the pattern and values of C(:,j) for a coarse Gustavson task;
// the pattern is not flagged as jumbled.

#if GB_IS_ANY_PAIR_SEMIRING

    // ANY_PAIR: result is purely symbolic; no numeric work to do
    #define GB_GATHER_ALL_C_j(mark)                                 \
        for (int64_t i = 0 ; i < cvlen ; i++)                       \
        {                                                           \
            if (Hf [i] == mark)                                     \
            {                                                       \
                Ci [pC++] = i ;                                     \
            }                                                       \
        }

#else

    // typical semiring
    #define GB_GATHER_ALL_C_j(mark)                                 \
        for (int64_t i = 0 ; i < cvlen ; i++)                       \
        {                                                           \
            if (Hf [i] == mark)                                     \
            {                                                       \
                GB_CIJ_GATHER (pC, i) ; /* Cx [pC] = Hx [i] */      \
                Ci [pC++] = i ;                                     \
            }                                                       \
        }

#endif

//------------------------------------------------------------------------------
// GB_SORT_C_j_PATTERN: sort C(:,j) for a coarse task, or flag as jumbled
//------------------------------------------------------------------------------

// Only coarse tasks do the optional sort.  Fine hash tasks always leave C
// jumbled.

#define GB_SORT_C_j_PATTERN                                     \
    if (do_sort)                                                \
    {                                                           \
        /* sort the pattern of C(:,j) (non-default) */          \
        GB_qsort_1 (Ci + Cp [kk], cjnz) ;                       \
    }                                                           \
    else                                                        \
    {                                                           \
        /* lazy sort: C(:,j) is now jumbled (default) */        \
        task_C_jumbled = true ;                                 \
    }

//------------------------------------------------------------------------------
// GB_SORT_AND_GATHER_C_j: sort and gather C(:,j) for a coarse Gustavson task
//------------------------------------------------------------------------------

// gather the values of C(:,j) for a coarse Gustavson task
#if GB_IS_ANY_PAIR_SEMIRING

    // ANY_PAIR: result is purely symbolic
    #define GB_SORT_AND_GATHER_C_j                              \
        GB_SORT_C_j_PATTERN ;

#else

    // typical semiring
    #define GB_SORT_AND_GATHER_C_j                              \
        GB_SORT_C_j_PATTERN ;                                   \
        /* gather the values into C(:,j) */                     \
        for (int64_t pC = Cp [kk] ; pC < Cp [kk+1] ; pC++)      \
        {                                                       \
            const int64_t i = Ci [pC] ;                         \
            GB_CIJ_GATHER (pC, i) ;   /* Cx [pC] = Hx [i] */    \
        }

#endif

//------------------------------------------------------------------------------
// GB_SORT_AND_GATHER_HASHED_C_j: sort and gather C(:,j) for a coarse hash task
//------------------------------------------------------------------------------

#if GB_IS_ANY_PAIR_SEMIRING

    // ANY_PAIR: result is purely symbolic
    #define GB_SORT_AND_GATHER_HASHED_C_j(hash_mark)                    \
        GB_SORT_C_j_PATTERN ;

#else

    // gather the values of C(:,j) for a coarse hash task
    #define GB_SORT_AND_GATHER_HASHED_C_j(hash_mark)                    \
        GB_SORT_C_j_PATTERN ;                                           \
        for (int64_t pC = Cp [kk] ; pC < Cp [kk+1] ; pC++)              \
        {                                                               \
            const int64_t i = Ci [pC] ;                                 \
            for (GB_HASH (i))           /* find i in hash table */      \
            {                                                           \
                if (Hf [hash] == (hash_mark) && (Hi [hash] == i))       \
                {                                                       \
                    /* i found in the hash table */                     \
                    /* Cx [pC] = Hx [hash] ; */                         \
                    GB_CIJ_GATHER (pC, hash) ;                          \
                    break ;                                             \
                }                                                       \
            }                                                           \
        }

#endif

//------------------------------------------------------------------------------
// GB_ATOMIC_UPDATE_HX:  Hx [i] += t
//------------------------------------------------------------------------------

#if GB_IS_ANY_MONOID

    //--------------------------------------------------------------------------
    // The update Hx [i] += t can be skipped entirely, for the ANY monoid.
    //--------------------------------------------------------------------------

    #define GB_ATOMIC_UPDATE_HX(i,t)

#elif GB_HAS_ATOMIC

    //--------------------------------------------------------------------------
    // Hx [i] += t via atomic update
    //--------------------------------------------------------------------------

    // for built-in MIN/MAX monoids only, on built-in types
    #define GB_MINMAX(i,t,done)                                     \
    {                                                               \
        GB_CTYPE xold, xnew, *px = Hx + (i) ;                       \
        do                                                          \
        {                                                           \
            /* xold = Hx [i] via atomic read */                     \
            GB_ATOMIC_READ                                          \
            xold = (*px) ;                                          \
            /* done if xold <= t for MIN, or xold >= t for MAX, */  \
            /* but not done if xold is NaN */                       \
            if (done) break ;                                       \
            xnew = t ;  /* t should be assigned; it is not NaN */   \
        }                                                           \
        while (!GB_ATOMIC_COMPARE_EXCHANGE (px, xold, xnew)) ;      \
    }

    #if GB_IS_IMIN_MONOID

        // built-in MIN monoids for signed and unsigned integers
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
            GB_MINMAX (i, t, xold <= t)

    #elif GB_IS_IMAX_MONOID

        // built-in MAX monoids for signed and unsigned integers
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
            GB_MINMAX (i, t, xold >= t)

    #elif GB_IS_FMIN_MONOID

        // built-in MIN monoids for float and double, with omitnan behavior.
        // The update is skipped entirely if t is NaN.  Otherwise, if t is not
        // NaN, xold is checked.  If xold is NaN, islessequal (xold, t) is
        // always false, so the non-NaN t must be always be assigned to Hx [i].
        // If both terms are not NaN, then islessequal (xold,t) is just
        // xold <= t.  If that is true, there is no work to do and
        // the loop breaks.  Otherwise, t is smaller than xold and so it must
        // be assigned to Hx [i].
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
        {                                                           \
            if (!isnan (t))                                         \
            {                                                       \
                GB_MINMAX (i, t, islessequal (xold, t)) ;           \
            }                                                       \
        }

    #elif GB_IS_FMAX_MONOID

        // built-in MAX monoids for float and double, with omitnan behavior.
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
        {                                                           \
            if (!isnan (t))                                         \
            {                                                       \
                GB_MINMAX (i, t, isgreaterequal (xold, t)) ;        \
            }                                                       \
        }

    #elif GB_IS_PLUS_FC32_MONOID

        // built-in PLUS_FC32 monoid can be done as two independent atomics
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
            GB_ATOMIC_UPDATE                                        \
            Hx_real [2*(i)] += crealf (t) ;                         \
            GB_ATOMIC_UPDATE                                        \
            Hx_imag [2*(i)] += cimagf (t) ;

    #elif GB_IS_PLUS_FC64_MONOID

        // built-in PLUS_FC64 monoid can be done as two independent atomics
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
            GB_ATOMIC_UPDATE                                        \
            Hx_real [2*(i)] += creal (t) ;                          \
            GB_ATOMIC_UPDATE                                        \
            Hx_imag [2*(i)] += cimag (t) ;

    #elif GB_HAS_OMP_ATOMIC

        // built-in PLUS and TIMES for integers and real, and boolean LOR,
        // LAND, LXOR monoids can be implemented with an OpenMP pragma.
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
            GB_ATOMIC_UPDATE                                        \
            GB_HX_UPDATE (i, t)

    #else

        // all other atomic monoids (EQ, XNOR) on boolean, signed and unsigned
        // integers, float, and double (not used for single and double
        // complex).
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
        {                                                           \
            GB_CTYPE xold, xnew, *px = Hx + (i) ;                   \
            do                                                      \
            {                                                       \
                /* xold = Hx [i] via atomic read */                 \
                GB_ATOMIC_READ                                      \
                xold = (*px) ;                                      \
                /* xnew = xold + t */                               \
                xnew = GB_ADD_FUNCTION (xold, t) ;                  \
            }                                                       \
            while (!GB_ATOMIC_COMPARE_EXCHANGE (px, xold, xnew)) ;  \
        }

    #endif

#else

    //--------------------------------------------------------------------------
    // Hx [i] += t can only be done inside the critical section
    //--------------------------------------------------------------------------

    // all user-defined monoids go here, and all complex monoids (except PLUS)
    #define GB_ATOMIC_UPDATE_HX(i,t)    \
        GB_OMP_FLUSH                    \
        GB_HX_UPDATE (i, t) ;           \
        GB_OMP_FLUSH

#endif

#define GB_IS_MINMAX_MONOID \
    (GB_IS_IMIN_MONOID || GB_IS_IMAX_MONOID ||  \
     GB_IS_FMIN_MONOID || GB_IS_FMAX_MONOID)

//------------------------------------------------------------------------------
// GB_ATOMIC_WRITE_HX:  Hx [i] = t
//------------------------------------------------------------------------------

#if GB_IS_ANY_PAIR_SEMIRING

    //--------------------------------------------------------------------------
    // ANY_PAIR: result is purely symbolic; no numeric work to do
    //--------------------------------------------------------------------------

    #define GB_ATOMIC_WRITE_HX(i,t)

#elif GB_HAS_ATOMIC

    //--------------------------------------------------------------------------
    // Hx [i] = t via atomic write
    //--------------------------------------------------------------------------

    #if GB_IS_PLUS_FC32_MONOID

        // built-in PLUS_FC32 monoid
        #define GB_ATOMIC_WRITE_HX(i,t)         \
            GB_ATOMIC_WRITE                     \
            Hx_real [2*(i)] = crealf (t) ;      \
            GB_ATOMIC_WRITE                     \
            Hx_imag [2*(i)] = cimagf (t) ;

    #elif GB_IS_PLUS_FC64_MONOID

        // built-in PLUS_FC64 monoid
        #define GB_ATOMIC_WRITE_HX(i,t)         \
            GB_ATOMIC_WRITE                     \
            Hx_real [2*(i)] = creal (t) ;       \
            GB_ATOMIC_WRITE                     \
            Hx_imag [2*(i)] = cimag (t) ;

    #else

        // all other atomic monoids
        #define GB_ATOMIC_WRITE_HX(i,t)         \
            GB_ATOMIC_WRITE                     \
            GB_HX_WRITE (i, t)

    #endif

#else

    //--------------------------------------------------------------------------
    // Hx [i] = t via critical section
    //--------------------------------------------------------------------------

    #define GB_ATOMIC_WRITE_HX(i,t)             \
        GB_OMP_FLUSH                            \
        GB_HX_WRITE (i, t) ;                    \
        GB_OMP_FLUSH

#endif

//------------------------------------------------------------------------------
// hash iteration
//------------------------------------------------------------------------------

// to iterate over the hash table, looking for index i:
// 
//      for (GB_HASH (i))
//      {
//          ...
//      }
//
// which expands into the following, where f(i) is the GB_HASHF(i) hash
// function:
//
//      for (int64_t hash = f(i) ; ; hash = (hash+1)&(hash_size-1))
//      {
//          ...
//      }

#define GB_HASH(i) \
    int64_t hash = GB_HASHF (i) ; ; GB_REHASH (hash,i)

//------------------------------------------------------------------------------
// define macros for any sparsity of A and B
//------------------------------------------------------------------------------

#undef GB_META16
#include "GB_meta16_definitions.h"

#endif

