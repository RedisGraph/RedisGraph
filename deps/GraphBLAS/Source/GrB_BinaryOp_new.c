//------------------------------------------------------------------------------
// GrB_BinaryOp_new: create a new user-defined binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GrB_BinaryOp_new is implemented both as a macro and a function.  Both are
// user-callable.  The macro is used by default since it can capture the name
// of the binary function.

#include "GB.h"

// the macro version of this function must first be #undefined
#undef GrB_BinaryOp_new
#undef GrM_BinaryOp_new

GrB_Info GRB (BinaryOp_new)
(
    GrB_BinaryOp *binaryop,         // handle for the new binary operator
    GxB_binary_function function,   // pointer to the binary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    GrB_Type ytype                  // type of input y
)
{ 
    return (GB_BinaryOp_new (binaryop, function, ztype, xtype, ytype, NULL)) ;
}

