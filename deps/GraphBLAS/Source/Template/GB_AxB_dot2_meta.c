//------------------------------------------------------------------------------
// GB_AxB_dot2_meta: C=A'*B, C<M>=A'*B or C<!M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#define GB_DOT2

#include "GB_unused.h"
#include "GB_AxB_dot_cij.h"

{

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    // A and B are never hypersparse.  If they are hypersparse on input, they
    // are converted to hyper_shallow form first, and the C matrix has smaller
    // dimensions.  The C bitmap/full matrix is converted back into a sparse or
    // hypersparse matrix when done.

    int64_t cnvals = 0 ;

    ASSERT (GB_IS_BITMAP (C) || GB_IS_FULL (C)) ;
    int8_t *restrict Cb = C->b ;
    const int64_t cvlen = C->vlen ;
    const bool C_is_full = (Cb == NULL) ;

    const int64_t *restrict Bp = B->p ;
    const int8_t  *restrict Bb = B->b ;
    const int64_t *restrict Bi = B->i ;
    const bool B_is_bitmap = GB_IS_BITMAP (B) ;
    const bool B_is_sparse = GB_IS_SPARSE (B) ;
    const bool B_is_full   = GB_as_if_full (B) ;
    const bool B_iso = B->iso ;
    ASSERT (!GB_IS_HYPERSPARSE (B)) ;
    #define B_is_hyper false

    const int64_t *restrict Ap = A->p ;
    const int8_t  *restrict Ab = A->b ;
    const int64_t *restrict Ai = A->i ;

    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_is_sparse = GB_IS_SPARSE (A) ;
    const bool A_is_full   = GB_as_if_full (A) ;
    const bool A_iso = A->iso ;
    ASSERT (!GB_IS_HYPERSPARSE (A)) ;
    #define A_is_hyper false

    #if !GB_A_IS_PATTERN
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    #endif
    #if !GB_B_IS_PATTERN
    const GB_BTYPE *restrict Bx = (GB_BTYPE *) B->x ;
    #endif
    #if !GB_IS_ANY_PAIR_SEMIRING
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif

    const int64_t vlen = A->vlen ;

    const int ntasks = naslice * nbslice ;

    //--------------------------------------------------------------------------
    // C=A'*B, C<M>=A'*B, or C<!M>=A'*B via dot products
    //--------------------------------------------------------------------------

    if (M == NULL)
    { 

        //----------------------------------------------------------------------
        // C = A'*B or C=A*B
        //----------------------------------------------------------------------

        #undef GB_MASK_IS_PRESENT

        if (A_not_transposed)
        {
            // C=A*B where A is bitmap or full, and B is sparse
            #define GB_A_NOT_TRANSPOSED
            ASSERT (A_is_bitmap || GB_IS_FULL (A)) ;
            ASSERT (B_is_sparse) ;
            if (C_is_full)
            {
                // C=A*B via dot products, where A is full and B is sparse,
                // and C is full
                ASSERT (GB_as_if_full (A)) ;
                #undef  GB_C_IS_FULL
                #define GB_C_IS_FULL   1
                #define GB_A_IS_SPARSE 0
                #define GB_A_IS_HYPER  0
                #define GB_A_IS_BITMAP 0
                #define GB_A_IS_FULL   1
                #define GB_B_IS_SPARSE 1
                #define GB_B_IS_HYPER  0
                #define GB_B_IS_BITMAP 0
                #define GB_B_IS_FULL   0
                #include "GB_AxB_dot2_template.c"
            }
            else if (A_is_bitmap)
            { 
                // C=A*B via dot products, where A is bitmap and B is sparse,
                // and C is bitmap
                #undef  GB_C_IS_FULL
                #define GB_C_IS_FULL   0
                #define GB_A_IS_SPARSE 0
                #define GB_A_IS_HYPER  0
                #define GB_A_IS_BITMAP 1
                #define GB_A_IS_FULL   0
                #define GB_B_IS_SPARSE 1
                #define GB_B_IS_HYPER  0
                #define GB_B_IS_BITMAP 0
                #define GB_B_IS_FULL   0
                #include "GB_AxB_dot2_template.c"
            }
            else
            { 
                // C=A*B via dot products, where A is full and B is sparse,
                // and C is bitmap
                #undef  GB_C_IS_FULL
                #define GB_C_IS_FULL   0
                #define GB_A_IS_SPARSE 0
                #define GB_A_IS_HYPER  0
                #define GB_A_IS_BITMAP 0
                #define GB_A_IS_FULL   1
                #define GB_B_IS_SPARSE 1
                #define GB_B_IS_HYPER  0
                #define GB_B_IS_BITMAP 0
                #define GB_B_IS_FULL   0
                #include "GB_AxB_dot2_template.c"
            } 
            #undef GB_A_NOT_TRANSPOSED
        }
        else if (C_is_full)
        {
            // C = A'*B, via dot2 method, where A is implicitly transposed,
            // C is full.  3 cases:
            #undef  GB_C_IS_FULL
            #define GB_C_IS_FULL   1
            if (A_is_full && B_is_full)
            {
                // A full, B full
                #define GB_A_IS_SPARSE 0
                #define GB_A_IS_HYPER  0
                #define GB_A_IS_BITMAP 0
                #define GB_A_IS_FULL   1
                #define GB_B_IS_SPARSE 0
                #define GB_B_IS_HYPER  0
                #define GB_B_IS_BITMAP 0
                #define GB_B_IS_FULL   1
                #include "GB_AxB_dot2_template.c"
            }
            else if (A_is_full)
            { 
                // A full, B sparse
                ASSERT (B_is_sparse) ;
                #define GB_A_IS_SPARSE 0
                #define GB_A_IS_HYPER  0
                #define GB_A_IS_BITMAP 0
                #define GB_A_IS_FULL   1
                #define GB_B_IS_SPARSE 1
                #define GB_B_IS_HYPER  0
                #define GB_B_IS_BITMAP 0
                #define GB_B_IS_FULL   0
                #include "GB_AxB_dot2_template.c"
            }
            else
            { 
                // A sparse, B full
                ASSERT (A_is_sparse) ;
                ASSERT (B_is_full) ;
                #define GB_A_IS_SPARSE 1
                #define GB_A_IS_HYPER  0
                #define GB_A_IS_BITMAP 0
                #define GB_A_IS_FULL   0
                #define GB_B_IS_SPARSE 0
                #define GB_B_IS_HYPER  0
                #define GB_B_IS_BITMAP 0
                #define GB_B_IS_FULL   1
                #include "GB_AxB_dot2_template.c"
            }
            #undef  GB_C_IS_FULL
            #define GB_C_IS_FULL   0
        }
        else
        {
            // C = A'*B, via dot2 method, where A is implicitly transposed,
            // C is bitmap
            #include "GB_meta16_factory.c"
        }

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

        const int8_t *restrict Mb = M->b ;
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
            // C<#M>=A'*B or C<#!M>=A*B
            //------------------------------------------------------------------

            const GB_void *restrict Mx = (GB_void *)
                (Mask_struct ? NULL : (M->x)) ;
            const size_t msize = M->type->size ;

            #define GB_MASK_IS_PRESENT

            if (A_not_transposed)
            {
                // C<#M>=A*B where A is bitmap or full, and B is sparse
                #define GB_A_NOT_TRANSPOSED
                ASSERT (A_is_bitmap || GB_IS_FULL (A)) ;
                ASSERT (B_is_sparse) ;
                if (A_is_bitmap)
                { 
                    // C<#M>=A*B via dot products, A is bitmap and B is sparse
                    #define GB_A_IS_SPARSE 0
                    #define GB_A_IS_HYPER  0
                    #define GB_A_IS_BITMAP 1
                    #define GB_A_IS_FULL   0
                    #define GB_B_IS_SPARSE 1
                    #define GB_B_IS_HYPER  0
                    #define GB_B_IS_BITMAP 0
                    #define GB_B_IS_FULL   0
                    #include "GB_AxB_dot2_template.c"
                }
                else
                { 
                    // C<#M>=A*B via dot products, A is full and B is sparse
                    #define GB_A_IS_SPARSE 0
                    #define GB_A_IS_HYPER  0
                    #define GB_A_IS_BITMAP 0
                    #define GB_A_IS_FULL   1
                    #define GB_B_IS_SPARSE 1
                    #define GB_B_IS_HYPER  0
                    #define GB_B_IS_BITMAP 0
                    #define GB_B_IS_FULL   0
                    #include "GB_AxB_dot2_template.c"
                } 
                #undef GB_A_NOT_TRANSPOSED
            }
            else
            {
                // C<#>M = A'*B, via dot2 method, A is implicitly transposed
                #include "GB_meta16_factory.c"
            }

            #undef GB_MASK_IS_PRESENT
        }
    }

    C->nvals = cnvals ;
}

#undef A_is_hyper
#undef B_is_hyper
#undef GB_DOT2

