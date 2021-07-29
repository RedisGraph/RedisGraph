//------------------------------------------------------------------------------
// GrB_UnaryOp_new: create a new user-defined unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GrB_UnaryOp_new is implemented both as a macro and a function.  Both are
// user-callable.  The macro is used by default since it can capture the name
// of the unary function.

#include "GB.h"

// the macro version of this function must first be #undefined
#undef GrB_UnaryOp_new
#undef GrM_UnaryOp_new

GrB_Info GRB (UnaryOp_new)          // create a new user-defined unary operator
(
    GrB_UnaryOp *unaryop,           // handle for the new unary operator
    GxB_unary_function function,    // pointer to the unary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype                  // type of input x
)
{ 
    return (GB_UnaryOp_new (unaryop, function, ztype, xtype, NULL)) ;
}

