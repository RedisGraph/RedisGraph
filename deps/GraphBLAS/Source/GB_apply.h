//------------------------------------------------------------------------------
// GB_apply.h: definitions for GB_apply
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_APPLY_H
#define GB_APPLY_H
#include "GB.h"

GrB_Info GB_apply                   // C<M> = accum (C, op(A)) or op(A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // M descriptor
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    const GrB_Matrix A,             // first input:  matrix A
    bool A_transpose,               // A matrix descriptor
    GB_Context Context
) ;

// Cx and A->x may be aliased in GB_apply_op

GrB_Info GB_apply_op                // apply a unary operator, Cx = op (A)
(
    GB_void *Cx,                    // output array, of type op->ztype
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,Ax) else binop(Ax,y)
    const GrB_Matrix A,             // input matrix
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_shallow_op      // create shallow matrix and apply operator
(
    GrB_Matrix *Chandle,    // output matrix C, of type op*->ztype
    const bool C_is_csc,    // desired CSR/CSC format of C
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    const GrB_Matrix A,     // input matrix to typecast
    GB_Context Context
) ;

#endif
