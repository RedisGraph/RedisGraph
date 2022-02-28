//------------------------------------------------------------------------------
// GxB_Matrix_Option_set: set an option in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_transpose.h"

#define GB_FREE_ALL ;

GrB_Info GxB_Matrix_Option_set      // set an option in a matrix
(
    GrB_Matrix A,                   // descriptor to modify
    GxB_Option_Field field,         // option to change
    ...                             // value to change it to
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GB_WHERE (A, "GxB_Matrix_Option_set (A, field, value)") ;
    GB_BURBLE_START ("GxB_set") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    ASSERT_MATRIX_OK (A, "A to set option", GB0) ;

    //--------------------------------------------------------------------------
    // set the matrix option
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {

        case GxB_HYPER_SWITCH : 

            {
                va_start (ap, field) ;
                double hyper_switch = va_arg (ap, double) ;
                va_end (ap) ;
                A->hyper_switch = (float) hyper_switch ;
            }
            break ;

        case GxB_BITMAP_SWITCH : 

            {
                va_start (ap, field) ;
                double bitmap_switch = va_arg (ap, double) ;
                va_end (ap) ;
                A->bitmap_switch = (float) bitmap_switch ;
            }
            break ;

        case GxB_SPARSITY_CONTROL : 

            {
                va_start (ap, field) ;
                int sparsity_control = va_arg (ap, int) ;
                va_end (ap) ;
                A->sparsity_control =
                    GB_sparsity_control (sparsity_control, (int64_t) (-1)) ;
            }
            break ;

        case GxB_FORMAT : 

            {
                va_start (ap, field) ;
                int format = va_arg (ap, int) ;
                va_end (ap) ;
                if (! (format == GxB_BY_ROW || format == GxB_BY_COL))
                { 
                    return (GrB_INVALID_VALUE) ;
                }
                // the value is normally GxB_BY_ROW (0) or GxB_BY_COL (1), but
                // any nonzero value results in GxB_BY_COL.
                bool new_csc = (format != GxB_BY_ROW) ;
                // conform the matrix to the new by-row/by-col format
                if (A->is_csc != new_csc)
                { 
                    // A = A', done in-place, and change to the new format.
                    GB_BURBLE_N (GB_nnz (A), "(transpose) ") ;
                    GB_OK (GB_transpose_in_place (A, new_csc, Context)) ;
                    ASSERT (A->is_csc == new_csc) ;
                    ASSERT (GB_JUMBLED_OK (A)) ;
                }
            }
            break ;

        default : 

            return (GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // conform the matrix to its new desired sparsity structure
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A set before conform", GB0) ;
    GB_OK (GB_conform (A, Context)) ;
    GB_BURBLE_END ;
    ASSERT_MATRIX_OK (A, "A set after conform", GB0) ;
    return (GrB_SUCCESS) ;
}

