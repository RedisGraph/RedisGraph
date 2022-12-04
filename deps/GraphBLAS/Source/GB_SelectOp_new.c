//------------------------------------------------------------------------------
// GB_SelectOp_new: create a new user-defined select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This function is historical.  Use GrB_IndexUnaryOp_new with GrB_select,
// instead of a user-defined GxB_SelectOp with GxB_select.  No JIT acceleration
// will be provided for user-defined GxB_SelectOps, so the operator definition
// string is not provided.

// The select function signature must be:

//      bool f (GrB_Index i, GrB_Index j, GrB_Index nrows, GrB_Index ncols,
//              const void *x, const void *thunk) ;

#include "GB.h"

GrB_Info GB_SelectOp_new            // create a new user-defined select operator
(
    GxB_SelectOp *selectop,         // handle for the new select operator
    GxB_select_function function,   // pointer to the select function
    GrB_Type xtype,                 // type of input x, or NULL if type-generic
    GrB_Type ttype,                 // type of input thunk, or NULL if not used
    const char *unused              // no longer used
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_SelectOp_new (selectop, function, xtype, ttype)") ;
    GB_RETURN_IF_NULL (selectop) ;
    (*selectop) = NULL ;
    GB_RETURN_IF_NULL (function) ;
    GB_RETURN_IF_FAULTY (xtype) ;   // xtype may be NULL
    GB_RETURN_IF_FAULTY (ttype) ;   // ttype may be NULL

    //--------------------------------------------------------------------------
    // create the select op
    //--------------------------------------------------------------------------

    // allocate the select operator
    size_t header_size ;
    (*selectop) = GB_MALLOC (1, struct GB_SelectOp_opaque, &header_size) ;
    if (*selectop == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    // initialize the select operator
    GxB_SelectOp op = *selectop ;
    op->magic = GB_MAGIC ;
    op->header_size = header_size ;
    op->ztype = GrB_BOOL ;
    op->xtype = xtype ;
    op->ytype = ttype ;         // thunk type

    op->unop_function = NULL ;
    op->idxunop_function = NULL ;
    op->binop_function = NULL ;
    op->selop_function = function ;

    op->opcode = GB_USER_selop_code ;
    memset (op->name, 0, GxB_MAX_NAME_LEN) ;
    snprintf (op->name, GxB_MAX_NAME_LEN-1, "user_selectop") ;
    op->defn = NULL ;           // unused: no JIT acceleration for these ops

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_SELECTOP_OK (op, "new user-defined select op", GB0) ;
    return (GrB_SUCCESS) ;
}

