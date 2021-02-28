//------------------------------------------------------------------------------
// gb_mxstring_to_selectop: get a GraphBLAS select operator from a MATLAB string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GxB_SelectOp gb_mxstring_to_selectop    // return select operator from a string
(
    const mxArray *mxstring             // MATLAB string
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (gb_mxarray_is_empty (mxstring), "invalid selectop") ;

    //--------------------------------------------------------------------------
    // get the string
    //--------------------------------------------------------------------------

    #define LEN 256
    char opstring [LEN+2] ;
    gb_mxstring_to_string (opstring, LEN, mxstring, "select operator") ;

    //--------------------------------------------------------------------------
    // convert the string to a select operator
    //--------------------------------------------------------------------------

    return (gb_string_to_selectop (opstring)) ;
}

