//------------------------------------------------------------------------------
// gb_mxstring_to_selectop: get GraphBLAS select operator from a built-in string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

// Returns a GrB_IndexUnaryOp or a GxB_SelectOp

#include "gb_interface.h"

void gb_mxstring_to_selectop
(
    // outputs: one of the outputs is non-NULL and the other NULL
    GrB_IndexUnaryOp *idxunop,          // GrB_IndexUnaryOp, if found
    GxB_SelectOp *selop,                // GxB_SelectOp if found
    bool *thunk_required,               // true if op requires a thunk scalar
    bool *op_is_positional,             // true if op is positional
    // input/output:
    int64_t *ithunk,
    // inputs:
    const mxArray *mxstring,            // built-in string
    const GrB_Type atype                // type of A, or NULL if not present
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

    gb_string_to_selectop (idxunop, selop, thunk_required, op_is_positional,
        ithunk, opstring, atype) ;
}

