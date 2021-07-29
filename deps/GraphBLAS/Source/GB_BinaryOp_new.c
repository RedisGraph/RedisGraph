//------------------------------------------------------------------------------
// GB_BinaryOp_new: create a new user-defined binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Create a new a binary operator: z = f (x,y).  The binary function signature
// must be void f (void *z, const void *x, const void *y), and then it must
// recast its input and output arguments internally as needed.

// This function is not directly user-callable.  Use GrB_BinaryOp_new instead.

#include "GB.h"
#include "GB_binop.h"

GrB_Info GB_BinaryOp_new
(
    GrB_BinaryOp *op,               // handle for the new binary operator
    GxB_binary_function function,   // pointer to the binary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    GrB_Type ytype,                 // type of input y
    const char *name                // name of the function (may be NULL)
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GrB_BinaryOp_new (op, function, ztype, xtype, ytype)") ;
    GB_RETURN_IF_NULL (op) ;
    (*op) = NULL ;
    GB_RETURN_IF_NULL (function) ;
    GB_RETURN_IF_NULL_OR_FAULTY (ztype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (xtype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (ytype) ;

    //--------------------------------------------------------------------------
    // allocate the binary op
    //--------------------------------------------------------------------------

    size_t header_size ;
    (*op) = GB_MALLOC (1, struct GB_BinaryOp_opaque, &header_size) ;
    if (*op == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }
    (*op)->header_size = header_size ;

    //--------------------------------------------------------------------------
    // create the binary op
    //--------------------------------------------------------------------------

    GB_binop_new (*op, function, ztype, xtype, ytype, name, GB_USER_opcode) ;
    return (GrB_SUCCESS) ;
}

