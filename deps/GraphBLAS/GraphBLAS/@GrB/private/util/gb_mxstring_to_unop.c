//------------------------------------------------------------------------------
// gb_mxstring_to_unop: get a GraphBLAS unary operator from a MATLAB string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_UnaryOp gb_mxstring_to_unop         // return unary operator from a string
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
        ERROR ("unary operator missing") ;
    }

    //--------------------------------------------------------------------------
    // get the string
    //--------------------------------------------------------------------------

    #define LEN 256
    char opstring [LEN+2] ;
    gb_mxstring_to_string (opstring, LEN, mxstring, "unary operator") ;

    //--------------------------------------------------------------------------
    // convert the string to a unary operator
    //--------------------------------------------------------------------------

    return (gb_string_to_unop (opstring, default_type)) ;
}

