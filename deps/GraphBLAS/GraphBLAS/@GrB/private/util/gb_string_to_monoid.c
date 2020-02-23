//------------------------------------------------------------------------------
// gb_string_to_monoid: get a GraphBLAS monoid from a string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

// The string has the form op_name.op_type.  For example '+.double' is
// GxB_PLUS_FP64_MONOID.  The type is optional.  If not present, it defaults
// to the default_type parameter.

GrB_Monoid gb_string_to_monoid          // return monoid from a string
(
    char *opstring,                     // string defining the operator
    const GrB_Type default_type         // default type if not in the string
)
{

    //--------------------------------------------------------------------------
    // get the binary operator and convert to a monoid
    //--------------------------------------------------------------------------

    return (gb_binop_to_monoid (gb_string_to_binop (opstring, default_type))) ;
}

