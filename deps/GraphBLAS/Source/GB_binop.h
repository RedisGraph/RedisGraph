//------------------------------------------------------------------------------
// GB_binop.h: definitions for binary operators
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_BINOP_H
#define GB_BINOP_H

bool GB_binop_builtin               // true if binary operator is builtin
(
    // inputs:
    const GrB_Type A_type,
    const bool A_is_pattern,        // true if only the pattern of A is used
    const GrB_Type B_type,
    const bool B_is_pattern,        // true if only the pattern of B is used
    const GrB_BinaryOp op,          // binary operator; may be NULL
    const bool flipxy,              // true if z=op(y,x), flipping x and y
    // outputs, unused by caller if this function returns false
    GB_Opcode *opcode,              // opcode for the binary operator
    GB_Type_code *xcode,            // type code for x input
    GB_Type_code *ycode,            // type code for y input
    GB_Type_code *zcode             // type code for z output
) ;

GB_Opcode GB_flip_opcode    // flipped opcode, or -1 on error
(
    GB_Opcode opcode,       // opcode to flip
    bool *handled           // true if opcode is handled by flipping the opcode
) ;

GrB_BinaryOp GB_flip_op     // flip a binary operator, or NULL on error
(
    GrB_BinaryOp op,        // binary operator to flip
    bool *handled           // true if operator is handled
) ;

GB_PUBLIC
GB_Opcode GB_boolean_rename     // renamed opcode
(
    const GB_Opcode opcode      // opcode to rename
) ;

GrB_BinaryOp GB_boolean_rename_op   // return renamed op
(
    const GrB_BinaryOp op           // op to rename
) ;

void GB_binop_new
(
    GrB_BinaryOp op,                // new binary operator
    GxB_binary_function function,   // binary function (may be NULL)
    GrB_Type ztype,                 // type of output z
    GrB_Type xtype,                 // type of input x
    GrB_Type ytype,                 // type of input y
    const char *name,               // name of the function (may be NULL)
    const GB_Opcode opcode          // opcode for the function
) ;

GrB_Monoid GB_binop_to_monoid       // return the corresponding monoid, or NULL
(
    const GrB_BinaryOp op_in        // binary op to convert
) ;

void GB_binop_rename            // rename a bound binary op
(
    GrB_UnaryOp *op1,           // set to new unary op, if op2 is renamed
    GrB_BinaryOp *op2,          // set to NULL if op2 is renamed
    bool binop_bind1st
) ;

void GB_binop_pattern
(
    // outputs:
    bool *A_is_pattern,     // true if A is pattern-only, because of the op
    bool *B_is_pattern,     // true if B is pattern-only, because of the op
    // inputs:
    const bool flipxy,      // if true,  z = op (b,a) will be computed
                            // if false, z = op (a,b) will be computed
    const GB_Opcode opcode  // opcode of binary op
) ;

#endif

