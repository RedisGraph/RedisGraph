//------------------------------------------------------------------------------
// gb_mxstring_to_selectop: get GraphBLAS select operator from a built-in string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

#include "gb_interface.h"

GxB_SelectOp gb_mxstring_to_selectop    // return select operator from a string
(
    const mxArray *mxstring             // built-in string
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

