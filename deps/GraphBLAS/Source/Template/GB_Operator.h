//------------------------------------------------------------------------------
// GB_operator.h: definitions of all operator objects
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GrB_UnaryOp, GrB_IndexUnaryOp, GrB_BinaryOp, and GxB_SelectOp all use the
// same internal structure.

    int64_t magic ;         // for detecting uninitialized objects
    size_t header_size ;    // size of the malloc'd block for this struct, or 0

    GrB_Type ztype ;        // type of z
    GrB_Type xtype ;        // type of x
    GrB_Type ytype ;        // type of y for binop and IndexUnaryOp,
                            // thunk for SelectOp.  NULL for unaryop

    // function pointers:
    GxB_unary_function       unop_function ;
    GxB_index_unary_function idxunop_function ;
    GxB_binary_function      binop_function ;
    GxB_select_function      selop_function ;

    char name [GxB_MAX_NAME_LEN] ;       // name of the unary operator
    GB_Opcode opcode ;      // operator opcode
    char *defn ;            // function definition
    size_t defn_size ;      // size of the definition

