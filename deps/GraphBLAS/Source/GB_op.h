//------------------------------------------------------------------------------
// GB_op.h: definitions for operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_OPERATOR_H
#define GB_OPERATOR_H

GrB_Info GB_Op_free             // free a user-created op
(
    GB_Operator *op_handle      // handle of operator to free
) ;

GB_PUBLIC
bool GB_op_is_second    // return true if op is SECOND, of the right type
(
    GrB_BinaryOp op,
    GrB_Type type
) ;

GrB_Info GB_op_name_and_defn
(
    // output
    char *operator_name,        // op->name of the GrB operator struct
    char **operator_defn,       // op->defn of the GrB operator struct
    size_t *operator_defn_size, // op->defn_size of the GrB operator struct
    // input
    const char *input_name,     // user-provided name, may be NULL
    const char *input_defn,     // user-provided name, may be NULL
    const char *typecast_name,  // typecast name for function pointer
    size_t typecast_name_len    // length of typecast_name
) ;

GrB_UnaryOp GB_unop_one (GB_Type_code xcode) ;

#endif

