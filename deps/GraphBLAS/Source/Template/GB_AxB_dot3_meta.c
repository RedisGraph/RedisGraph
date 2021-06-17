//------------------------------------------------------------------------------
// GB_AxB_dot3_meta: C<M>=A'*B via dot products, where C is sparse/hypersparse
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#define GB_DOT3
#define GB_DOT3_PHASE2

#include "GB_unused.h"
#include "GB_AxB_dot_cij.h"

// GB_DOT_ALWAYS_SAVE_CIJ: C(i,j) = cij
#if GB_CIJ_CHECK

    #define GB_DOT_ALWAYS_SAVE_CIJ      \
    {                                   \
        cij_exists = true ;             \
        GB_PUTC (cij, pC) ;             \
        Ci [pC] = i ;                   \
    }

#else

    #define GB_DOT_ALWAYS_SAVE_CIJ      \
    {                                   \
        GB_PUTC (cij, pC) ;             \
        Ci [pC] = i ;                   \
    }

#endif

// GB_DOT_SAVE_CIJ: C(i,j) = cij, if it exists
#define GB_DOT_SAVE_CIJ             \
{                                   \
    if (GB_CIJ_EXISTS)              \
    {                               \
        GB_PUTC (cij, pC) ;         \
        Ci [pC] = i ;               \
    }                               \
}

{

    //--------------------------------------------------------------------------
    // get M, A, B, and C
    //--------------------------------------------------------------------------

    // C and M have the same sparsity patter (both are sparse or hyper),
    // except entries of C may become zombies.  M is not complemented.

    int64_t nzombies = 0 ;

    ASSERT (GB_IS_SPARSE (C) || GB_IS_HYPERSPARSE (C)) ;
    const int64_t *GB_RESTRICT Cp = C->p ;
    const int64_t *GB_RESTRICT Ch = C->h ;
    int64_t  *GB_RESTRICT Ci = C->i ;
    GB_CTYPE *GB_RESTRICT Cx = (GB_CTYPE *) C->x ;
    const int64_t cvlen = C->vlen ;

    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bh = B->h ;
    const int8_t  *GB_RESTRICT Bb = B->b ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    const GB_BTYPE *GB_RESTRICT Bx = (GB_BTYPE *) (B_is_pattern ? NULL : B->x) ;
    const int64_t bnvec = B->nvec ;
    const bool B_is_hyper = GB_IS_HYPERSPARSE (B) ;
    const bool B_is_bitmap = GB_IS_BITMAP (B) ;
    const bool B_is_sparse = GB_IS_SPARSE (B) ;

    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int8_t  *GB_RESTRICT Ab = A->b ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    const GB_ATYPE *GB_RESTRICT Ax = (GB_ATYPE *) (A_is_pattern ? NULL : A->x) ;
    const int64_t anvec = A->nvec ;
    const bool A_is_hyper = GB_IS_HYPERSPARSE (A) ;
    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_is_sparse = GB_IS_SPARSE (A) ;

    const int64_t vlen = A->vlen ;
    ASSERT (A->vlen == B->vlen) ;

    const bool M_is_sparse = GB_IS_SPARSE (M) ;
    ASSERT (M_is_sparse || GB_IS_HYPERSPARSE (M)) ;
    const int64_t *GB_RESTRICT Mi = M->i ;
    const size_t mvlen = M->vlen ;

    //--------------------------------------------------------------------------
    // C<M> = A'*B via dot products, where C and M are both sparse/hyper
    //--------------------------------------------------------------------------

    // 4 possible cases of the mask are handled:

    // M can be sparse or hyper, and always present
    // M can be structural or valued
    // M is not complemented

    // The other 12 cases of the mask, and the one no-mask case, are handled
    // by dot2.

    if (M_is_sparse && Mask_struct && A_is_sparse && B_is_sparse)
    {
        // special case: M is sparse and structural, and A and B are sparse
        #define GB_MASK_SPARSE_AND_STRUCTURAL
        #define GB_A_IS_SPARSE 1
        #define GB_A_IS_HYPER  0
        #define GB_A_IS_BITMAP 0
        #define GB_A_IS_FULL   0
        #define GB_B_IS_SPARSE 1
        #define GB_B_IS_HYPER  0
        #define GB_B_IS_BITMAP 0
        #define GB_B_IS_FULL   0
        #include "GB_AxB_dot3_template.c"
        #undef GB_MASK_SPARSE_AND_STRUCTURAL
    }
    else
    {
        // general case
        const GB_void *GB_RESTRICT Mx = (GB_void *)
            (Mask_struct ? NULL : (M->x)) ;
        const size_t msize = M->type->size ;
        #include "GB_meta16_factory.c"
    }

    C->nzombies = nzombies ;
}

#undef GB_DOT_ALWAYS_SAVE_CIJ
#undef GB_DOT_SAVE_CIJ

#undef GB_DOT3
#undef GB_DOT3_PHASE2

