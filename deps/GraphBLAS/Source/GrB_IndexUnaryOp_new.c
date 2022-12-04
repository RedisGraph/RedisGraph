//------------------------------------------------------------------------------
// GrB_IndexUnaryOp_new: create a new user-defined index_unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GrB_IndexUnaryOp_new is implemented both as a macro and a function.  Both are
// user-callable.  The macro is used by default since it can capture the name
// of the index_unary function.

#include "GB.h"

// the macro version of this function must first be #undefined
#undef GrB_IndexUnaryOp_new
#undef GrM_IndexUnaryOp_new

GrB_Info GRB (IndexUnaryOp_new)     // create a new user-defined IndexUnary op
(
    GrB_IndexUnaryOp *op,           // handle for the new IndexUnary operator
    GxB_index_unary_function function,    // pointer to IndexUnary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x (the A(i,j) entry)
    GrB_Type ytype                  // type of input y (the scalar)
)
{ 
    return (GxB_IndexUnaryOp_new (op, function, ztype, xtype, ytype,
        NULL, NULL)) ;
}

