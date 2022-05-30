//------------------------------------------------------------------------------
// GB_AxB_saxpy4_template.c: C+=A*B when C is full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This method is only used for built-in semirings with no typecasting.
// The accumulator matches the semiring monoid.
// The ANY monoid and non-atomic monoids are not supported.

// C is as-if-full.
// B is bitmap or full.
// A is sparse or hypersparse.

#undef  GB_FREE_ALL
#define GB_FREE_ALL                         \
{                                           \
    GB_FREE_WORK (&Wf, Wf_size) ;           \
    GB_FREE_WORK (&Wcx, Wcx_size) ;         \
    GB_WERK_POP (H_slice, int64_t) ;        \
    GB_WERK_POP (B_slice, int64_t) ;        \
}

#undef  GB_C_IS_BITMAP
#define GB_C_IS_BITMAP 0

#if !GB_HAS_ATOMIC
#error "saxpy4 not defined for non-atomic monoids"
#endif

#if GB_IS_ANY_MONOID
#error "saxpy4 not defined for the ANY monoid"
#endif

{

    //--------------------------------------------------------------------------
    // declare workspace
    //--------------------------------------------------------------------------

    int8_t  *restrict Wf  = NULL ; size_t Wf_size = 0 ;
    GB_void *restrict Wcx = NULL ; size_t Wcx_size = 0 ;
    GB_WERK_DECLARE (H_slice, int64_t) ;
    GB_WERK_DECLARE (B_slice, int64_t) ;

    //--------------------------------------------------------------------------
    // get C, M, A, and B
    //--------------------------------------------------------------------------

    ASSERT (GB_as_if_full (C)) ;                 // C is always full
    const int64_t cvlen = C->vlen ;
    ASSERT (C->vlen == A->vlen) ;
    ASSERT (C->vdim == B->vdim) ;
    ASSERT (A->vdim == B->vlen) ;

    const int8_t *restrict Bb = B->b ;
    const bool B_iso = B->iso ;
    const int64_t bvlen = B->vlen ;
    const int64_t bvdim = B->vdim ;
    const bool B_is_bitmap = GB_IS_BITMAP (B) ;
    ASSERT (B_is_bitmap || GB_as_if_full (B)) ;

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ah = A->h ;
    const int64_t *restrict Ai = A->i ;
    const bool A_iso = A->iso ;
    const int64_t anvec = A->nvec ;
    const int64_t avlen = A->vlen ;
    const int64_t avdim = A->vdim ;
    ASSERT (GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A)) ;

    #if !GB_A_IS_PATTERN
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    #endif
    #if !GB_B_IS_PATTERN
    const GB_BTYPE *restrict Bx = (GB_BTYPE *) B->x ;
    #endif
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;

    //--------------------------------------------------------------------------
    // C += A*B, no mask, A sparse/hyper, B bitmap/full
    //--------------------------------------------------------------------------

    #define GB_NO_MASK 1
    #define GB_MASK_IS_SPARSE_OR_HYPER 0
    #define GB_MASK_IS_BITMAP_OR_FULL  0
    if (B_is_bitmap)
    { 
        // A is sparse/hyper, B is bitmap, no mask
        #undef  GB_B_IS_BITMAP
        #define GB_B_IS_BITMAP 1
        #include "GB_bitmap_AxB_saxpy_A_sparse_B_bitmap_template.c"
    }
    else
    { 
        // A is sparse/hyper, B is full, no mask
        #undef  GB_B_IS_BITMAP
        #define GB_B_IS_BITMAP 0
        #include "GB_bitmap_AxB_saxpy_A_sparse_B_bitmap_template.c"
    }
    #undef GB_MASK_IS_SPARSE_OR_HYPER
    #undef GB_MASK_IS_BITMAP_OR_FULL
    #undef GB_NO_MASK
    #undef GB_B_IS_BITMAP

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
}

#undef GB_FREE_ALL
#undef GB_C_IS_BITMAP

