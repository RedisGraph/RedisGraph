//------------------------------------------------------------------------------
// gb_mxstring_to_binop: get a GraphBLAS operator from a MATLAB string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_BinaryOp gb_mxstring_to_binop       // return binary operator from a string
(
    const mxArray *mxstring,            // MATLAB string
    const GrB_Type default_type         // default type if not in the string
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (gb_mxarray_is_empty (mxstring))
    { 
        // no operator is present, or present and empty; this is not yet an
        // error, since many uses of GraphBLAS functions use an optional accum
        // operator.
        return (NULL) ;
    }

    //--------------------------------------------------------------------------
    // get the string
    //--------------------------------------------------------------------------

    #define LEN 256
    char opstring [LEN+2] ;
    gb_mxstring_to_string (opstring, LEN, mxstring, "binary operator") ;

    //--------------------------------------------------------------------------
    // convert the string to a binary operator
    //--------------------------------------------------------------------------

    return (gb_string_to_binop (opstring, default_type)) ;
}

