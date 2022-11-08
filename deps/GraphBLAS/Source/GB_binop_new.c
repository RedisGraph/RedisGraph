//------------------------------------------------------------------------------
// GB_binop_new: create a new operator (user-defined or internal)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Create a new a binary operator: z = f (x,y).  The function pointer may
// be NULL, for implied functions (FIRST and SECOND).  It may not be NULL
// otherwise.

// The binary op header is allocated by the caller, and passed in
// uninitialized.

#include "GB.h"
#include "GB_binop.h"

GrB_Info GB_binop_new
(
    GrB_BinaryOp op,                // new binary operator
    GxB_binary_function function,   // binary function (may be NULL)
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    GrB_Type ytype,                 // type of input y
    const char *binop_name,         // name of the user function
    const char *binop_defn,         // definition of the user function
    const GB_Opcode opcode          // opcode for the function
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (op != NULL) ;
    ASSERT (ztype != NULL) ;
    ASSERT (xtype != NULL) ;
    ASSERT (ytype != NULL) ;
    ASSERT (GB_IS_BINARYOP_CODE (opcode)) ;

    //--------------------------------------------------------------------------
    // initialize the binary operator
    //--------------------------------------------------------------------------

    op->magic = GB_MAGIC ;
    op->ztype = ztype ;
    op->xtype = xtype ;
    op->ytype = ytype ;
    op->unop_function = NULL ;
    op->idxunop_function = NULL ;
    op->binop_function = function ;       // may be NULL
    op->selop_function = NULL ;
    op->opcode = opcode ;

    //--------------------------------------------------------------------------
    // get the binary op name and defn
    //--------------------------------------------------------------------------

    return (GB_op_name_and_defn (op->name, &(op->defn), &(op->defn_size),
        binop_name, binop_defn, "GxB_binary_function", 19)) ;
}

