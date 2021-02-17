//------------------------------------------------------------------------------
// GB_AxB_dot2_meta: C=A'*B, C<M>=A'*B or C<!M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#define GB_DOT2

#include "GB_unused.h"
#include "GB_AxB_dot_cij.h"

// GB_DOT_ALWAYS_SAVE_CIJ: C(i,j) = cij
#define GB_DOT_ALWAYS_SAVE_CIJ      \
{                                   \
    GB_PUTC (cij, pC) ;             \
    Cb [pC] = 1 ;                   \
    task_cnvals++ ;                 \
}

// GB_DOT_SAVE_CIJ: C(i,j) = cij, unless already done by GB_DOT
#if GB_IS_ANY_MONOID

    // for the ANY monoid, GB_DOT saves C(i,j) as soon as a value is found
    #define GB_DOT_SAVE_CIJ

#else

    // all other monoids: C(i,j) = cij if it exists
    #define GB_DOT_SAVE_CIJ             \
    {                                   \
        if (GB_CIJ_EXISTS)              \
        {                               \
            GB_DOT_ALWAYS_SAVE_CIJ ;    \
        }                               \
    }

#endif

{

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    // A and B are never hypersparse.  If they are hypersparse on input, they
    // are converted to packed sparse form first, and the C matrix has smaller
    // dimensions.  The C bitmap matrix is unpacked into a sparse or
    // hypersparse matrix when done.

    int64_t cnvals = 0 ;

    ASSERT (GB_IS_BITMAP (C)) ;
    int8_t   *GB_RESTRICT Cb = C->b ;
    GB_CTYPE *GB_RESTRICT Cx = (GB_CTYPE *) C->x ;
    const int64_t cvlen = C->vlen ;

    const int64_t *GB_RESTRICT Bp = B->p ;
    const int8_t  *GB_RESTRICT Bb = B->b ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    const GB_BTYPE *GB_RESTRICT Bx = (GB_BTYPE *) (B_is_pattern ? NULL : B->x) ;
    const bool B_is_bitmap = GB_IS_BITMAP (B) ;
    const bool B_is_sparse = GB_IS_SPARSE (B) ;
    ASSERT (!GB_IS_HYPERSPARSE (B)) ;
    #define B_is_hyper false

    const int64_t *GB_RESTRICT Ap = A->p ;
    const int8_t  *GB_RESTRICT Ab = A->b ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    const GB_ATYPE *GB_RESTRICT Ax = (GB_ATYPE *) (A_is_pattern ? NULL : A->x) ;
    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_is_sparse = GB_IS_SPARSE (A) ;
    ASSERT (!GB_IS_HYPERSPARSE (A)) ;
    #define A_is_hyper false

    const int64_t vlen = A->vlen ;
    ASSERT (A->vlen == B->vlen) ;

    const int ntasks = naslice * nbslice ;

    //--------------------------------------------------------------------------
    // C=A'*B, C<M>=A'*B, or C<!M>=A'*B via dot products
    //--------------------------------------------------------------------------

    if (M == NULL)
    { 

        //----------------------------------------------------------------------
        // C = A'*B
        //----------------------------------------------------------------------

        #undef GB_MASK_IS_PRESENT
        #include "GB_meta16_factory.c"

    }
    else
    {

        //----------------------------------------------------------------------
        // C<M>=A'*B or C<!M>=A'*B
        //----------------------------------------------------------------------

        // 12 possible cases of the mask are handled:

        // if M is not complemented (Mask_comp is false): 4 cases
        // M can be bitmap or full, not sparse or hyper (dot3 handles that)
        // M can be structural or valued

        // if M is complemented (Mask_comp is true): 8 cases
        // M can be sparse, hyper, bitmap, or full
        // M can be structural or valued

        const int8_t *GB_RESTRICT Mb = M->b ;
        const bool M_is_bitmap = GB_IS_BITMAP (M) ;
        const bool M_is_full = GB_IS_FULL (M) ;

        #if ( GB_IS_ANY_MONOID )
        if (B_is_bitmap && A_is_sparse && M_is_bitmap && Mask_struct
            && Mask_comp)
        {

            //------------------------------------------------------------------
            // C<#M,struct> = A'*B, special case
            //------------------------------------------------------------------

            // GB_ANY_SPECIALIZED is defined if the following conditions hold:
            // semirings: all built-in semirings with the ANY monoid
            // A: sparse
            // B: bitmap
            // M: bitmap
            // Mask_comp: true
            // Mask_struct: true

            GBURBLE ("(specialized) ") ;
            #define GB_ANY_SPECIALIZED
            #define GB_MASK_IS_PRESENT
            #define GB_A_IS_SPARSE 1
            #define GB_A_IS_HYPER  0
            #define GB_A_IS_BITMAP 0
            #define GB_A_IS_FULL   0
            #define GB_B_IS_SPARSE 0
            #define GB_B_IS_SPARSE 0
            #define GB_B_IS_BITMAP 1
            #define GB_B_IS_FULL   0
            #include "GB_AxB_dot2_template.c"
            #undef  GB_ANY_SPECIALIZED
            #undef GB_MASK_IS_PRESENT

        }
        else
        #endif
        { 

            //------------------------------------------------------------------
            // C<M>=A'*B or C<!M>=A'*B
            //------------------------------------------------------------------

            const GB_void *GB_RESTRICT Mx = (GB_void *)
                (Mask_struct ? NULL : (M->x)) ;
            const size_t msize = M->type->size ;

            #define GB_MASK_IS_PRESENT
            #include "GB_meta16_factory.c"
            #undef GB_MASK_IS_PRESENT

        }
    }

    C->nvals = cnvals ;
}

#undef A_is_hyper
#undef B_is_hyper

#undef GB_DOT_ALWAYS_SAVE_CIJ
#undef GB_DOT_SAVE_CIJ

#undef GB_DOT2

