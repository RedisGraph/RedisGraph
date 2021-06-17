//------------------------------------------------------------------------------
// GB_mex_triu: compute C=triu(A,1)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C = triu (A,k), where A and C are double

#include "GB_mex.h"

#define USAGE "C = GB_mex_triu (A,k)"

#define FREE_ALL                        \
{                                       \
    GxB_Scalar_free_(&Thunk) ;          \
    GrB_Matrix_free_(&A) ;              \
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

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Matrix A = NULL, C = NULL ;
    GxB_Scalar Thunk = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 1 || nargin > 2)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    int64_t k = 0 ;
    // get k
    if (nargin > 1)
    {
        k = (int64_t) mxGetScalar (pargin [1]) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // construct C
    METHOD (GrB_Matrix_new (&C, GrB_FP64, A->vlen, A->vdim)) ;

    #undef GET_DEEP_COPY
    #undef FREE_DEEP_COPY

    #define GET_DEEP_COPY  GrB_Matrix_new (&C, GrB_FP64, A->vlen, A->vdim) ;
    #define FREE_DEEP_COPY GrB_Matrix_free_(&C) ;

    GxB_Scalar_new (&Thunk, GrB_INT64) ;
    GxB_Scalar_setElement_INT64_(Thunk, k) ;
    GxB_Scalar_wait_(&Thunk) ;

    // C = triu (A,k)
    METHOD (GxB_Matrix_select_(C, NULL, NULL, GxB_TRIU, A, Thunk, NULL)) ;

    // return C to MATLAB as a regular MATLAB sparse matrix
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C triu", false) ;

    FREE_ALL ;
}

