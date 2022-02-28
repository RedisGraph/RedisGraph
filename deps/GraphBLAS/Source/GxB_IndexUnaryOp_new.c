//------------------------------------------------------------------------------
// GxB_IndexUnaryOp_new: create a new user-defined index_unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Create a new a index_unary operator: z = f (x,i,j,thunk).  The
// index_unary function signature must be:

// void f (void *z, const void *x, int64_t i, int64_t j, const void *thunk)

// and then it must recast its inputs (x and thunk) and output (z) arguments
// internally as needed.  When used with a GrB_Vector, j is zero.

#include "GB.h"

GrB_Info GxB_IndexUnaryOp_new       // create a new user-defined index_unary op
(
    GrB_IndexUnaryOp *op,           // handle for the new GrB_IndexUnaryOp
    GxB_index_unary_function function,   // pointer to the index_unary function
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    GrB_Type ttype,                 // type of input thunk
    const char *idxop_name,         // name of the user function
    const char *idxop_defn          // definition of the user function
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_IndexUnaryOp_new (op, function, ztype, xtype, ttype"
        ", name, defn)") ;
    GB_RETURN_IF_NULL (op) ;
    (*op) = NULL ;
    GB_RETURN_IF_NULL (function) ;
    GB_RETURN_IF_NULL_OR_FAULTY (ztype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (xtype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (ttype) ;

    //--------------------------------------------------------------------------
    // allocate the index_unary op
    //--------------------------------------------------------------------------

    size_t header_size ;
    (*op) = GB_MALLOC (1, struct GB_IndexUnaryOp_opaque, &header_size) ;
    if (*op == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }
    (*op)->header_size = header_size ;

    //--------------------------------------------------------------------------
    // initialize the index_unary operator
    //--------------------------------------------------------------------------

    (*op)->magic = GB_MAGIC ;
    (*op)->ztype = ztype ;
    (*op)->xtype = xtype ;
    (*op)->ytype = ttype ;      // thunk type

    (*op)->unop_function = NULL ;
    (*op)->idxunop_function = function ;
    (*op)->binop_function = NULL ;
    (*op)->selop_function = NULL ;

    (*op)->opcode = GB_USER_idxunop_code ;
    // get the index_unary op name and defn
    GB_op_name_and_defn ((*op)->name, &((*op)->defn), idxop_name, idxop_defn,
        "GxB_index_unary_function", 24) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_INDEXUNARYOP_OK ((*op), "new user-defined index_unary op", GB0) ;
    return (GrB_SUCCESS) ;
}

