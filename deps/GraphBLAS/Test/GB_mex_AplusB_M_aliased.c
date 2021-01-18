//------------------------------------------------------------------------------
// GB_mex_AplusB_M_aliased: compute C<B>=A+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This is for testing only.  See GrB_eWiseAdd instead.  Returns a plain MATLAB
// matrix, in double.

#include "GB_mex.h"

#define USAGE "C = GB_mex_AplusB_M_aliased (A, B, op)"

#define FREE_ALL                        \
{                                       \
    GrB_Matrix_free_(&A) ;              \
    GrB_Matrix_free_(&B) ;              \
    GrB_Matrix_free_(&C) ;              \
    GB_mx_put_global (true) ;           \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    // double tic2 [2] ;
    // simple_tic (tic2) ;

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;
    GrB_Matrix C = NULL ;
    GrB_BinaryOp op = NULL ;

    GB_CONTEXT (USAGE) ;

    // check inputs
    if (nargout > 1 || nargin != 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A and B
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, true) ;
    B = GB_mx_mxArray_to_Matrix (pargin [1], "B", false, true) ;
    if (A == NULL || B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    // get op
    bool user_complex = (Complex != GxB_FC64)
        && (A->type == Complex || B->type == Complex) ;
    if (!GB_mx_mxArray_to_BinaryOp (&op, pargin [2], "op",
        A->type, user_complex) || op == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("op failed") ;
    }

    // C<B> = A+B using the op.  M == B alias
    bool ignore ;
    METHOD (GB_add (&C, A->type, true, B, false, false, &ignore, A, B, op,
        Context)) ;

    // return C to MATLAB as a plain sparse matrix
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C<B>=A+B result", false) ;

    FREE_ALL ;
}

