//------------------------------------------------------------------------------
// GB_mex_isequal: returns true if A and B are equal
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "C = GB_mex_isequal (A, B)"

#define FREE_ALL                        \
{                                       \
    GrB_Matrix_free_(&A) ;               \
    GrB_Matrix_free_(&B) ;               \
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
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;

    // check inputs
    if (nargout > 1 || nargin != 2)
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

    // C = all (A == B) ; if type is Complex and Complex != GxB_FC64,
    // use Complex_eq
    bool result ;
    METHOD (isequal (&result, A, B, Complex_eq)) ;

    // return C to MATLAB as a plain sparse matrix
    pargout [0] = mxCreateDoubleScalar ((double) result) ;

    FREE_ALL ;
}

