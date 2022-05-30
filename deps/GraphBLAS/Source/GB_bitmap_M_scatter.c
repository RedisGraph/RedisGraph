//------------------------------------------------------------------------------
// GB_bitmap_M_scatter: scatter M into/from the C bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_bitmap_assign_methods.h"

void GB_bitmap_M_scatter        // scatter M into the C bitmap
(
    // input/output:
    GrB_Matrix C,
    // inputs:
    const GrB_Index *I,         // I index list
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,         // J index list
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix M,         // mask to scatter into the C bitmap
    const bool Mask_struct,     // true if M is structural, false if valued
    const int assign_kind,      // row assign, col assign, assign, or subassign
    const int operation,        // +=2, -=2, or %=2
    const int64_t *M_ek_slicing,    // size 3*M_ntasks+1
    const int M_ntasks,
    const int M_nthreads,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (M, "M for bitmap scatter", GB0) ;
    ASSERT (GB_IS_SPARSE (M) || GB_IS_HYPERSPARSE (M)) ;
    ASSERT (M_ntasks > 0) ;
    ASSERT (M_nthreads > 0) ;
    ASSERT (M_ek_slicing != NULL) ;

    //--------------------------------------------------------------------------
    // get C and M
    //--------------------------------------------------------------------------

    GB_GET_M
    int8_t *Cb = C->b ;
    const int64_t cvlen = C->vlen ;
    int64_t cnvals = 0 ;

    //--------------------------------------------------------------------------
    // scatter M into the C bitmap
    //--------------------------------------------------------------------------

    switch (operation)
    {

        case GB_BITMAP_M_SCATTER_PLUS_2 :       // Cb (i,j) += 2

            #undef  GB_MASK_WORK
            #define GB_MASK_WORK(pC) Cb [pC] += 2
            #include "GB_bitmap_assign_M_template.c"
            break ;

        case GB_BITMAP_M_SCATTER_MINUS_2 :      // Cb (i,j) -= 2

            #undef  GB_MASK_WORK
            #define GB_MASK_WORK(pC) Cb [pC] -= 2
            #include "GB_bitmap_assign_M_template.c"
            break ;

        case GB_BITMAP_M_SCATTER_MOD_2 :        // Cb (i,j) %= 2

            #undef  GB_MASK_WORK
            #define GB_MASK_WORK(pC) Cb [pC] %= 2
            #include "GB_bitmap_assign_M_template.c"
            break ;

        default: ;
    }
}

