//------------------------------------------------------------------------------
// GrB_UnaryOp_new: create a new user-defined unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_UnaryOp_new is implemented both as a macro and a function.  Both are
// user-callable.  The macro is used by default since it can capture the name
// of the unary function.

#include "GB.h"

// the macro version of this function must first be #undefined
#undef GrB_UnaryOp_new

GrB_Info GrB_UnaryOp_new            // create a new user-defined unary operator
(
    GrB_UnaryOp *unaryop,           // handle for the new unary operator
    GxB_unary_function function,    // pointer to the unary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype                  // type of input x
)
{ 
    return (GB_UnaryOp_new (unaryop, function, ztype, xtype, "f")) ;
}

