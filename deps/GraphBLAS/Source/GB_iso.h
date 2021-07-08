//------------------------------------------------------------------------------
// GB_iso.h: definitions for iso methods
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_ISO_H
#define GB_ISO_H

typedef enum
{
    GB_NON_ISO = 0,     // result is not iso
    GB_ISO_1 = 1,       // iso value is 1
    GB_ISO_S = 2,       // iso value is the scalar input
    GB_ISO_A = 3,       // iso value is A
    GB_ISO_OP1_A = 4,   // iso value is op1 (A)
    GB_ISO_OP2_SA = 5,  // iso value is op2 (scalar, A)
    GB_ISO_OP2_AS = 6   // iso value is op2 (A, scalar)
}
GB_iso_code ;

GB_iso_code GB_iso_unop_code
(
    GrB_Matrix A,           // input matrix
    GrB_UnaryOp op1,        // unary operator, if present
    GrB_BinaryOp op2,       // binary operator, if present
    bool binop_bind1st      // if true, C = op2(x,A), otherwise C = op2(A,y)
) ;

void GB_iso_unop            // Cx [0] = op1 (A), op2 (s,A) or op2 (A,s)
(
    // output
    GB_void *restrict Cx,   // output scalar of iso array
    // input
    GrB_Type ctype,         // type of Cx
    GB_iso_code C_code_iso, // defines how C iso value is to be computed
    GrB_UnaryOp op1,        // unary operator, if present
    GrB_BinaryOp op2,       // binary operator, if present
    GrB_Matrix A,           // input matrix
    GxB_Scalar scalar       // input scalar
) ;

GB_PUBLIC
GrB_Info GB_convert_any_to_non_iso // convert iso matrix to non-iso
(
    GrB_Matrix A,           // input/output matrix
    bool initialize,        // if true, copy the iso value to all of A->x
    GB_Context Contest
) ;

GB_PUBLIC
GrB_Info GB_convert_any_to_iso // convert non-iso matrix to iso
(
    GrB_Matrix A,           // input/output matrix
    GB_void *scalar,        // scalar value, of size A->type->size, or NULL
    bool compact,           // if true, also reduce the space for A->x
    GB_Context Context
) ;

void GB_iso_expand          // expand an iso scalar into an entire array
(
    void *restrict X,       // output array to expand into
    int64_t n,              // # of entries in X
    void *restrict scalar,  // scalar to expand into X
    size_t size,            // size of the scalar and each entry of X
    GB_Context Context
) ;

GB_PUBLIC
bool GB_iso_check               // return true if A is iso, false otherwise
(
    const GrB_Matrix A,         // matrix to reduce
    GB_Context Context
) ;

#endif

