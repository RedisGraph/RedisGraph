//------------------------------------------------------------------------------
// GB_transpose.h:  definitions for GB_transpose
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_TRANSPOSE_H
#define GB_TRANSPOSE_H
#include "GB.h"
#include "GB_atomics.h"

bool GB_transpose_method        // if true: use GB_builder, false: use bucket
(
    const GrB_Matrix A,         // matrix to transpose
    int *nworkspaces_bucket,    // # of slices of A for the bucket method
    int *nthreads_bucket,       // # of threads to use for the bucket method
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_transpose           // C=A', C=(ctype)A or C=op(A')
(
    GrB_Matrix *Chandle,        // output matrix C, possibly modified in-place
    GrB_Type ctype,             // desired type of C; if NULL use A->type.
                                // ignored if op is present (cast to op->ztype)
    const bool C_is_csc,        // desired CSR/CSC format of C
    const GrB_Matrix A_in,      // input matrix
        // no operator is applied if both op1 and op2 are NULL
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    GB_Context Context
) ;

GrB_Info GB_transpose_bucket    // bucket transpose; typecast and apply op
(
    GrB_Matrix *Chandle,        // output matrix (unallocated on input)
    const GrB_Type ctype,       // type of output matrix C
    const bool C_is_csc,        // format of output matrix C
    const GrB_Matrix A,         // input matrix
        // no operator is applied if both op1 and op2 are NULL
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    const int nworkspaces,      // # of workspaces to use
    const int nthreads,         // # of threads to use
    GB_Context Context
) ;

void GB_transpose_ix            // transpose the pattern and values of a matrix
(
    GrB_Matrix C,                       // output matrix
    const GrB_Matrix A,                 // input matrix
    // for sparse case:
    int64_t *GB_RESTRICT *Workspaces,   // Workspaces, size nworkspaces
    const int64_t *GB_RESTRICT A_slice, // how A is sliced, size nthreads+1
    int nworkspaces,                    // # of workspaces to use
    // for all cases:
    int nthreads                        // # of threads to use
) ;

void GB_transpose_op    // transpose, typecast, and apply operator to a matrix
(
    GrB_Matrix C,                       // output matrix
        // no operator is applied if both op1 and op2 are NULL
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    const GrB_Matrix A,                 // input matrix
    // for sparse or hypersparse case:
    int64_t *GB_RESTRICT *Workspaces,   // Workspaces, size nworkspaces
    const int64_t *GB_RESTRICT A_slice, // how A is sliced, size nthreads+1
    int nworkspaces,                    // # of workspaces to use
    // for all cases:
    int nthreads                        // # of threads to use
) ;

GB_PUBLIC   // accessed by the MATLAB interface only
GrB_Info GB_shallow_copy    // create a purely shallow matrix
(
    GrB_Matrix *Chandle,    // output matrix C
    const bool C_is_csc,    // desired CSR/CSC format of C
    const GrB_Matrix A,     // input matrix
    GB_Context Context
) ;

#endif

