//------------------------------------------------------------------------------
// GB_AxB_saxpy5_meta.c: C+=A*B when C is full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This method is only used for built-in semirings with no typecasting.
// The accumulator matches the semiring monoid.
// The ANY monoid is not supported.

// C is as-if-full.
// A is bitmap or full.
// B is sparse or hypersparse.

#if GB_IS_ANY_MONOID
#error "saxpy5 not defined for the ANY monoid"
#endif

{

    //--------------------------------------------------------------------------
    // get C, A, and B
    //--------------------------------------------------------------------------

    ASSERT (GB_as_if_full (C)) ;
    ASSERT (C->vlen == A->vlen) ;
    ASSERT (C->vdim == B->vdim) ;
    ASSERT (A->vdim == B->vlen) ;
    ASSERT (GB_IS_BITMAP (A) || GB_as_if_full (A)) ;
    ASSERT (GB_IS_SPARSE (B) || GB_IS_HYPERSPARSE (B)) ;

    const bool A_is_bitmap = GB_IS_BITMAP (A) ;

    //--------------------------------------------------------------------------
    // C += A*B, no mask, A bitmap/full, B sparse/hyper
    //--------------------------------------------------------------------------

    #if GB_A_IS_PATTERN
    {

        //----------------------------------------------------------------------
        // A is pattern-only
        //----------------------------------------------------------------------

        if (A_is_bitmap)
        {
            // A is bitmap and pattern-only
            #undef  GB_A_IS_BITMAP
            #define GB_A_IS_BITMAP 1
            #include "GB_AxB_saxpy5_iso_or_pattern.c"
        }
        else
        {
            // A is full and pattern-only
            #undef  GB_A_IS_BITMAP
            #define GB_A_IS_BITMAP 0
            #include "GB_AxB_saxpy5_iso_or_pattern.c"
        }

    }
    #else
    {

        //----------------------------------------------------------------------
        // A is valued
        //----------------------------------------------------------------------

        if (A->iso)
        {

            //------------------------------------------------------------------
            // A is iso-valued
            //------------------------------------------------------------------

            if (A_is_bitmap)
            { 
                // A is bitmap, iso-valued, B is sparse/hyper
                #undef  GB_A_IS_BITMAP
                #define GB_A_IS_BITMAP 1
                #include "GB_AxB_saxpy5_iso_or_pattern.c"
            }
            else
            { 
                // A is full, iso-valued, B is sparse/hyper
                #undef  GB_A_IS_BITMAP
                #define GB_A_IS_BITMAP 0
                #include "GB_AxB_saxpy5_iso_or_pattern.c"
            }

        }
        else
        {

            //------------------------------------------------------------------
            // general case: A is non-iso and valued
            //------------------------------------------------------------------

            if (A_is_bitmap)
            { 
                // A is bitmap, non-iso-valued, B is sparse/hyper
                #undef  GB_A_IS_BITMAP
                #define GB_A_IS_BITMAP 1
                #include "GB_AxB_saxpy5_bitmap.c"
                #undef  GB_A_IS_BITMAP
            }
            else
            { 
                // A is full, non-iso-valued, B is sparse/hyper
                #if GB_SEMIRING_HAS_AVX_IMPLEMENTATION          \
                    && GB_COMPILER_SUPPORTS_AVX512F             \
                    && GB_V4_512
                if (GB_Global_cpu_features_avx512f ( ))
                {
                    // x86_64 with AVX512f
                    GB_AxB_saxpy5_unrolled_avx512f (C, A, B,
                        ntasks, nthreads, B_slice, Context) ;
                    return (GrB_SUCCESS) ;
                }
                #endif
                #if GB_SEMIRING_HAS_AVX_IMPLEMENTATION          \
                    && GB_COMPILER_SUPPORTS_AVX2                \
                    && GB_V4_256
                if (GB_Global_cpu_features_avx2 ( ))
                {
                    // x86_64 with AVX2
                    GB_AxB_saxpy5_unrolled_avx2 (C, A, B,
                        ntasks, nthreads, B_slice, Context) ;
                    return (GrB_SUCCESS) ;
                }
                #endif
                // any architecture and any built-in semiring
                GB_AxB_saxpy5_unrolled_vanilla (C, A, B,
                    ntasks, nthreads, B_slice, Context) ;
            }
        }
    }
    #endif
}

#undef GB_A_IS_BITMAP
#undef GB_B_IS_HYPER

