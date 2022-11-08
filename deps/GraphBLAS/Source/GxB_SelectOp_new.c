//------------------------------------------------------------------------------
// GxB_SelectOp_new: create a new user-defined select operator (historical)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This function is historical.  Use GrB_IndexUnaryOp_new with GrB_select,
// instead of a user-defined GxB_SelectOp with GxB_select.

// The select function signature must be:

//      bool f (GrB_Index i, GrB_Index j, GrB_Index nrows, GrB_Index ncols,
//              const void *x, const void *thunk) ;

#include "GB.h"

#undef GxB_SelectOp_new
#undef GxM_SelectOp_new

GrB_Info GXB (SelectOp_new)         // create a new user-defined select operator
(
    GxB_SelectOp *selectop,         // handle for the new select operator
    GxB_select_function function,   // pointer to the select function
    GrB_Type xtype,                 // type of input x, or NULL if type-generic
    GrB_Type ttype                  // type of input thunk, or NULL if not used
)
{ 
    return (GB_SelectOp_new (selectop, function, xtype, ttype, NULL)) ;
}

