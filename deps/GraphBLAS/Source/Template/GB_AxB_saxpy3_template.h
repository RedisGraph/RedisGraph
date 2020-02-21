//------------------------------------------------------------------------------
// GB_AxB_saxpy3_template.h: C=A*B, C<M>=A*B, or C<!M>=A*B via saxpy3 method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Definitions for GB_AxB_saxpy3_template.c

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
    GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mpright,            \
        ((Bh == NULL) ? kk : Bh [kk]), &pM_start, &pM_end) ;    \
    int64_t mjnz = pM_end - pM_start ;    /* nnz (M (:,j)) */

//------------------------------------------------------------------------------
// GB_GET_M_j_RANGE: get the first and last indices in M(:,j)
//------------------------------------------------------------------------------

#define GB_GET_M_j_RANGE(gamma)                                 \
    int64_t im_first = -1, im_last = -1 ;                       \
    if (mjnz > 0)                                               \
    {                                                           \
        im_first = Mi [pM_start] ;  /* get first M(:,j) */      \
        im_last  = Mi [pM_end-1] ;  /* get last M(:,j) */       \
    }                                                           \
    int64_t mjnz_much = mjnz * gamma

//------------------------------------------------------------------------------
// GB_SCATTER_M_j: scatter M(:,j) for a fine or coarse Gustavson task
//------------------------------------------------------------------------------

#define GB_SCATTER_M_j_TYPE(mask_t,pMstart,pMend,mark)                  \
{                                                                       \
    const mask_t *GB_RESTRICT Mxx = (mask_t *) Mx ;                     \
    for (int64_t pM = pMstart ; pM < pMend ; pM++) /* scan M(:,j) */    \
    {                                                                   \
        if (Mxx [pM]) Hf [Mi [pM]] = mark ;   /* Hf [i] = M(i,j) */     \
    }                                                                   \
}                                                                       \
break ;

// scatter M(:,j) for a coarse Gustavson task, C<M>=A*B or C<!M>=A*B
#define GB_SCATTER_M_j(pMstart,pMend,mark)                                  \
    if (Mx == NULL)                                                         \
    {                                                                       \
        /* mask is structural, not valued */                                \
        for (int64_t pM = pMstart ; pM < pMend ; pM++)                      \
        {                                                                   \
            Hf [Mi [pM]] = mark ;   /* Hf [i] = M(i,j) */                   \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* mask is valued, not structural */                                \
        switch (msize)                                                      \
        {                                                                   \
            default:                                                        \
            case 1: GB_SCATTER_M_j_TYPE (uint8_t , pMstart, pMend, mark) ;  \
            case 2: GB_SCATTER_M_j_TYPE (uint16_t, pMstart, pMend, mark) ;  \
            case 4: GB_SCATTER_M_j_TYPE (uint32_t, pMstart, pMend, mark) ;  \
            case 8: GB_SCATTER_M_j_TYPE (uint64_t, pMstart, pMend, mark) ;  \
        }                                                                   \
    }

//------------------------------------------------------------------------------
// GB_HASH_M_j: scatter M(:,j) for a coarse hash task
//------------------------------------------------------------------------------

// hash M(:,j) into Hf and Hi for coarse hash task, C<M>=A*B or C<!M>=A*B
#define GB_HASH_M_j                                                     \
    for (int64_t pM = pM_start ; pM < pM_end ; pM++) /* scan M(:,j) */  \
    {                                                                   \
        GB_GET_M_ij ;           /* get M(i,j) */                        \
        if (!mij) continue ;    /* skip if M(i,j)=0 */                  \
        int64_t i = Mi [pM] ;                                           \
        for (GB_HASH (i))       /* find i in hash */                    \
        {                                                               \
            if (Hf [hash] < mark)                                       \
            {                                                           \
                Hf [hash] = mark ;  /* insert M(i,j)=1 */               \
                Hi [hash] = i ;                                         \
                break ;                                                 \
            }                                                           \
        }                                                               \
    }

//------------------------------------------------------------------------------
// GB_GET_B_j: prepare to iterate over B(:,j)
//------------------------------------------------------------------------------

// prepare to iterate over the vector B(:,j), the (kk)th vector in B,
// where j == ((Bh == NULL) ? kk : Bh [kk]).  Note that j itself is never
// needed; just kk.
#define GB_GET_B_j                                                          \
    int64_t pleft = 0 ;                                                     \
    int64_t pright = anvec-1 ;                                              \
    int64_t pB = Bp [kk] ;                                                  \
    int64_t pB_end = Bp [kk+1] ;                                            \
    int64_t bjnz = pB_end - pB ;  /* nnz (B (:,j) */                        \
    /* FUTURE::: can skip if mjnz == 0 for C<M>=A*B tasks */                \
    if (A_is_hyper && bjnz > 2)                                             \
    {                                                                       \
        /* trim Ah [0..pright] to remove any entries past last B(:,j), */   \
        /* to speed up GB_lookup in GB_GET_A_k. */                          \
        GB_bracket_right (Bi [pB_end-1], Ah, 0, &pright) ;                  \
    }

//------------------------------------------------------------------------------
// GB_GET_B_kj: get the numeric value of B(k,j)
//------------------------------------------------------------------------------

#define GB_GET_B_kj \
    GB_GETB (bkj, Bx, pB)       /* bkj = Bx [pB] */

//------------------------------------------------------------------------------
// GB_GET_A_k: prepare to iterate over the vector A(:,k)
//------------------------------------------------------------------------------

#define GB_GET_A_k                                                          \
    int64_t pA_start, pA_end ;                                              \
    GB_lookup (A_is_hyper, Ah, Ap, &pleft, pright, k, &pA_start, &pA_end) ; \
    int64_t aknz = pA_end - pA_start ;    /* nnz (A (:,k)) */

//------------------------------------------------------------------------------
// GB_SKIP_IF_A_k_DISJOINT_WITH_M_j:  skip if A(:,k) and M(:,j) are disjoint
//------------------------------------------------------------------------------

// skip C(:,j)<M> += A(:,k)*B(k,j) if A(:,k) and M(:,j), for C<M>=A*B methods
#define GB_SKIP_IF_A_k_DISJOINT_WITH_M_j                    \
    if (aknz == 0) continue ;                               \
    int64_t alo = Ai [pA_start] ;   /* get first A(:,k) */  \
    int64_t ahi = Ai [pA_end-1] ;   /* get last A(:,k) */   \
    if (ahi < im_first || alo > im_last) continue

//------------------------------------------------------------------------------
// GB_GET_M_ij: get the numeric value of M(i,j)
//------------------------------------------------------------------------------

#define GB_GET_M_ij                                 \
    /* get M(i,j), at Mi [pM] and Mx [pM] */        \
    bool mij = GB_mcast (Mx, pM, msize)

//------------------------------------------------------------------------------
// GB_MULT_A_ik_B_kj: declare t and compute t = A(i,k) * B(k,j)
//------------------------------------------------------------------------------

#if GB_IS_PAIR_MULTIPLIER

    // PAIR multiplier: t is always 1; no numeric work to do to compute t.
    // The LXOR_PAIR and PLUS_PAIR semirings need the value t = 1 to use in
    // their monoid operator, however.
    #define t 1
    #define GB_MULT_A_ik_B_kj

#else

    // typical semiring
    #define GB_MULT_A_ik_B_kj                                   \
        GB_GETA (aik, Ax, pA) ;     /* aik = Ax [pA] ;  */      \
        GB_CIJ_DECLARE (t) ;        /* ctype t ;        */      \
        GB_MULT (t, aik, bkj)       /* t = aik * bkj ;  */

#endif

//------------------------------------------------------------------------------
// GB_COMPUTE_DENSE_C_j: compute C(:,j)=A*B(:,j) when C(:,j) is completely dense
//------------------------------------------------------------------------------

#if GB_IS_ANY_PAIR_SEMIRING

    // ANY_PAIR: result is purely symbolic; no numeric work to do
    #define GB_COMPUTE_DENSE_C_j                                \
        for (int64_t i = 0 ; i < cvlen ; i++)                   \
        {                                                       \
            Ci [pC + i] = i ;                                   \
        }

#else

    // typical semiring
    #define GB_COMPUTE_DENSE_C_j                                    \
        for (int64_t i = 0 ; i < cvlen ; i++)                       \
        {                                                           \
            Ci [pC + i] = i ;                                       \
            GB_CIJ_WRITE (pC + i, GB_IDENTITY) ; /* C(i,j)=0 */     \
        }                                                           \
        for ( ; pB < pB_end ; pB++)     /* scan B(:,j) */           \
        {                                                           \
            int64_t k = Bi [pB] ;       /* get B(k,j) */            \
            GB_GET_A_k ;                /* get A(:,k) */            \
            if (aknz == 0) continue ;                               \
            GB_GET_B_kj ;               /* bkj = B(k,j) */          \
            /* FUTURE::: handle the case when A(:,k) is dense */    \
            /* scan A(:,k) */                                       \
            for (int64_t pA = pA_start ; pA < pA_end ; pA++)        \
            {                                                       \
                int64_t i = Ai [pA] ;    /* get A(i,k) */           \
                GB_MULT_A_ik_B_kj ;      /* t = A(i,k)*B(k,j) */    \
                GB_CIJ_UPDATE (pC + i, t) ; /* Cx [pC+i]+=t */      \
            }                                                       \
        }

#endif

//------------------------------------------------------------------------------
// GB_COMPUTE_C_j_WHEN_NNZ_B_j_IS_ONE: compute C(:,j) when nnz(B(:,j)) == 1
//------------------------------------------------------------------------------

// C(:,j) = A(:,k)*B(k,j) when there is a single entry in B(:,j)
#if GB_IS_ANY_PAIR_SEMIRING

    // ANY_PAIR: result is purely symbolic; no numeric work to do
    #define GB_COMPUTE_C_j_WHEN_NNZ_B_j_IS_ONE                      \
        int64_t k = Bi [pB] ;       /* get B(k,j) */                \
        GB_GET_A_k ;                /* get A(:,k) */                \
        memcpy (Ci + pC, Ai + pA_start, aknz * sizeof (int64_t)) ;

#else

    // typical semiring
    #define GB_COMPUTE_C_j_WHEN_NNZ_B_j_IS_ONE                      \
        int64_t k = Bi [pB] ;       /* get B(k,j) */                \
        GB_GET_A_k ;                /* get A(:,k) */                \
        GB_GET_B_kj ;               /* bkj = B(k,j) */              \
        /* scan A(:,k) */                                           \
        for (int64_t pA = pA_start ; pA < pA_end ; pA++)            \
        {                                                           \
            int64_t i = Ai [pA] ;       /* get A(i,k) */            \
            GB_MULT_A_ik_B_kj ;         /* t = A(i,k)*B(k,j) */     \
            GB_CIJ_WRITE (pC, t) ;      /* Cx [pC] = t */           \
            Ci [pC++] = i ;                                         \
        }

#endif

//------------------------------------------------------------------------------
// GB_GATHER_ALL_C_j: gather the values and pattern of C(:,j)
//------------------------------------------------------------------------------

// gather the pattern and values of C(:,j) for a coarse Gustavson task (no sort)
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
// GB_SORT_AND_GATHER_C_j: sort the pattern of C(:,j) and gather values
//------------------------------------------------------------------------------

// sort the pattern of C(:,j) then gather the values for a coarse Gustavson task
#if GB_IS_ANY_PAIR_SEMIRING

    // ANY_PAIR: result is purely symbolic; just sort the pattern
    #define GB_SORT_AND_GATHER_C_j                              \
        /* sort the pattern of C(:,j) */                        \
        GB_qsort_1a (Ci + Cp [kk], cjnz) ;

#else

    // typical semiring
    #define GB_SORT_AND_GATHER_C_j                              \
        /* sort the pattern of C(:,j) */                        \
        GB_qsort_1a (Ci + Cp [kk], cjnz) ;                      \
        /* gather the values into C(:,j) */                     \
        for (int64_t pC = Cp [kk] ; pC < Cp [kk+1] ; pC++)      \
        {                                                       \
            int64_t i = Ci [pC] ;                               \
            GB_CIJ_GATHER (pC, i) ;   /* Cx [pC] = Hx [i] */    \
        }

#endif

//------------------------------------------------------------------------------
// GB_SORT_AND_GATHER_HASHED_C_j: sort pattern, gather values, for coarse hash 
//------------------------------------------------------------------------------

#if GB_IS_ANY_PAIR_SEMIRING

    // ANY_PAIR: result is purely symbolic; just sort the pattern
    #define GB_SORT_AND_GATHER_HASHED_C_j(hash_mark,Hi_hash_equals_i)       \
        /* sort the pattern of C(:,j) */                                    \
        GB_qsort_1a (Ci + Cp [kk], cjnz) ;

#else

    // sort the pattern of C(:,j) then gather the values for a coarse hash task
    #define GB_SORT_AND_GATHER_HASHED_C_j(hash_mark,Hi_hash_equals_i)       \
        /* sort the pattern of C(:,j) */                                    \
        GB_qsort_1a (Ci + Cp [kk], cjnz) ;                                  \
        for (int64_t pC = Cp [kk] ; pC < Cp [kk+1] ; pC++)                  \
        {                                                                   \
            int64_t i = Ci [pC] ;                                           \
            int64_t marked = (hash_mark) ;                                  \
            for (GB_HASH (i))           /* find i in hash table */          \
            {                                                               \
                if (Hf [hash] == marked && (Hi_hash_equals_i))              \
                {                                                           \
                    /* i found in the hash table */                         \
                    /* Cx [pC] = Hx [hash] ; */                             \
                    GB_CIJ_GATHER (pC, hash) ;                              \
                    break ;                                                 \
                }                                                           \
            }                                                               \
        }

#endif

//------------------------------------------------------------------------------
// GB_SCAN_M_j_OR_A_k: compute C(:,j) using linear scan or binary search
//------------------------------------------------------------------------------

// C(:,j)<M(:,j)>=A(:,k)*B(k,j) using one of two methods
#define GB_SCAN_M_j_OR_A_k                                              \
{                                                                       \
    if (aknz > 256 && mjnz_much < aknz)                                 \
    /* nnz(M(:,j)) much less than nnz(A(:,k)) */                        \
    {                                                                   \
        /* scan M(:,j), and do binary search for A(i,k) */              \
        int64_t pA = pA_start ;                                         \
        for (int64_t pM = pM_start ; pM < pM_end ; pM++)                \
        {                                                               \
            GB_GET_M_ij ;           /* get M(i,j) */                    \
            if (!mij) continue ;    /* skip if M(i,j)=0 */              \
            int64_t i = Mi [pM] ;                                       \
            bool found ;            /* search for A(i,k) */             \
            int64_t apright = pA_end - 1 ;                              \
            GB_BINARY_SEARCH (i, Ai, pA, apright, found) ;              \
            if (found)                                                  \
            {                                                           \
                /* C(i,j)<M(i,j)> += A(i,k) * B(k,j) for this method. */\
                /* M(i,j) is now known to be equal to 1, so there are */\
                /* cases in the GB_IKJ operation that can never */      \
                /* occur.  This could be pruned from the GB_IKJ */      \
                /* operation, but then this operation would differ */   \
                /* from the GB_IKJ operation in the linear-time scan */ \
                /* of A(:,j), below.  It's unlikely that pruning this */\
                /* case would lead to much performance improvement. */  \
                GB_IKJ ;                                                \
            }                                                           \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        /* scan A(:,k), and lookup M(i,j) */                            \
        for (int64_t pA = pA_start ; pA < pA_end ; pA++)                \
        {                                                               \
            int64_t i = Ai [pA] ;    /* get A(i,k) */                   \
            /* do C(i,j)<M(i,j)> += A(i,k) * B(k,j) for this method */  \
            /* M(i,j) may be 0 or 1, as given in the hash table */      \
            GB_IKJ ;                                                    \
        }                                                               \
    }                                                                   \
}

//------------------------------------------------------------------------------
// GB_ATOMIC_UPDATE_HX:  Hx [i] += t
//------------------------------------------------------------------------------

#if GB_IS_ANY_MONOID

    // The update Hx [i] += t can be skipped entirely, for the ANY monoid.
    #define GB_ATOMIC_UPDATE_HX(i,t)

#elif GB_HAS_ATOMIC

    // Hx [i] += t via atomic update
    #if GB_HAS_OMP_ATOMIC

        // built-in PLUS, TIMES, LOR, LAND, LXOR monoids can be
        // implemented with an OpenMP pragma
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
            GB_ATOMIC_UPDATE                                        \
            GB_HX_UPDATE (i, t)

    #else

        // built-in MIN, MAX, and EQ monoids only, which cannot
        // be implemented with an OpenMP pragma
        #define GB_ATOMIC_UPDATE_HX(i,t)                            \
            GB_CTYPE xold, xnew, *px = Hx + (i) ;                   \
            do                                                      \
            {                                                       \
                /* xold = Hx [i] via atomic read */                 \
                GB_ATOMIC_READ                                      \
                xold = (*px) ;                                      \
                /* xnew = xold + t */                               \
                xnew = GB_ADD_FUNCTION (xold, t) ;                  \
            }                                                       \
            while (!__atomic_compare_exchange (px, &xold, &xnew,    \
                true, __ATOMIC_RELAXED, __ATOMIC_RELAXED))

    #endif

//          prior version:
//          while (!__sync_bool_compare_and_swap
//              ((GB_CTYPE_PUN *) px,
//              * ((GB_CTYPE_PUN *) (&xold)),
//              * ((GB_CTYPE_PUN *) (&xnew))))

#else

    // Hx [i] += t can only be done inside the critical section
    #define GB_ATOMIC_UPDATE_HX(i,t)       \
        GB_PRAGMA (omp flush)           \
        GB_HX_UPDATE (i, t) ;           \
        GB_PRAGMA (omp flush)

#endif

//------------------------------------------------------------------------------
// GB_ATOMIC_WRITE_HX:  Hx [i] = t
//------------------------------------------------------------------------------

#if GB_IS_ANY_PAIR_SEMIRING

    // ANY_PAIR: result is purely symbolic; no numeric work to do
    #define GB_ATOMIC_WRITE_HX(i,t)

#else 

    // atomic write
    #if GB_HAS_ATOMIC
        // Hx [i] = t via atomic write
        #define GB_ATOMIC_WRITE_HX(i,t)       \
            GB_ATOMIC_WRITE   \
            GB_HX_WRITE (i, t)
    #else
        // Hx [i] = t via critical section
        #define GB_ATOMIC_WRITE_HX(i,t)       \
            GB_PRAGMA (omp flush)          \
            GB_HX_WRITE (i, t) ;           \
            GB_PRAGMA (omp flush)
    #endif

#endif

//------------------------------------------------------------------------------
// hash
//------------------------------------------------------------------------------

// to iterate over the hash table, looking for index i:
// for (GB_HASH (i)) { ... }
#define GB_HASH(i) int64_t hash = GB_HASH_FUNCTION (i) ; ; GB_REHASH (hash,i)

#endif

//------------------------------------------------------------------------------
// free workspace
//------------------------------------------------------------------------------

#undef  GB_FREE_INITIAL_WORK
#define GB_FREE_INITIAL_WORK ;

#undef  GB_FREE_TASKLIST_AND_HASH_TABLES
#define GB_FREE_TASKLIST_AND_HASH_TABLES                                    \
{                                                                           \
    GB_FREE_MEMORY (*(TaskList_handle), ntasks, sizeof (GB_saxpy3task_struct));\
    GB_FREE_MEMORY (Hi_all, Hi_size_total, sizeof (int64_t)) ;              \
    GB_FREE_MEMORY (Hf_all, Hf_size_total, sizeof (int64_t)) ;              \
    GB_FREE_MEMORY (Hx_all, Hx_size_total, 1) ;                             \
}

#undef  GB_FREE_WORK
#define GB_FREE_WORK                                                        \
{                                                                           \
    GB_FREE_INITIAL_WORK ;                                                  \
    GB_FREE_TASKLIST_AND_HASH_TABLES ;                                      \
}

#undef  GB_FREE_ALL
#define GB_FREE_ALL                                                         \
{                                                                           \
    GB_FREE_WORK ;                                                          \
    GB_MATRIX_FREE (Chandle) ;                                              \
}

