//------------------------------------------------------------------------------
// GxB_Vector_Option_set: set an option in a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_transpose.h"

#define GB_FREE_ALL ;

GrB_Info GxB_Vector_Option_set      // set an option in a vector
(
    GrB_Vector v,                   // descriptor to modify
    GxB_Option_Field field,         // option to change
    ...                             // value to change it to
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info = GrB_SUCCESS ;
    GB_WHERE (v, "GxB_Vector_Option_set (v, field, value)") ;
    GB_BURBLE_START ("GxB_set (vector option)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;
    ASSERT_VECTOR_OK (v, "v to set option", GB0) ;

    //--------------------------------------------------------------------------
    // set the vector option
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {

        case GxB_BITMAP_SWITCH : 

            {
                va_start (ap, field) ;
                double bitmap_switch = va_arg (ap, double) ;
                va_end (ap) ;
                v->bitmap_switch = (float) bitmap_switch ;
            }
            break ;

        case GxB_SPARSITY_CONTROL : 

            {
                va_start (ap, field) ;
                int sparsity_control = va_arg (ap, int) ;
                va_end (ap) ;
                v->sparsity_control =
                    GB_sparsity_control (sparsity_control, (int64_t) (-1)) ;
            }
            break ;

        default : 

            return (GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // conform the vector to its new desired sparsity structure
    //--------------------------------------------------------------------------

    GB_OK (GB_conform ((GrB_Matrix) v, Context)) ;
    GB_BURBLE_END ;
    ASSERT_VECTOR_OK (v, "v set", GB0) ;
    return (info) ;
}

