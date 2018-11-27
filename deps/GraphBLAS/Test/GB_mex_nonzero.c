//------------------------------------------------------------------------------
// GB_mex_nonzero: compute C=nonzero(A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = nonzero (A), where A and C are double

#include "GB_mex.h"

#define USAGE "C = GB_mex_nonzero (A)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GB_mx_put_global (true, 0) ;        \
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

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin != 1)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // construct C
    METHOD (GrB_Matrix_new (&C, GrB_FP64, A->vlen, A->vdim)) ;

    #undef GET_DEEP_COPY
    #undef FREE_DEEP_COPY

    #define GET_DEEP_COPY  GrB_Matrix_new (&C, GrB_FP64, A->vlen, A->vdim) ;
    #define FREE_DEEP_COPY GrB_free (&C) ;

    // C = nonzero (A)
    METHOD (GxB_Matrix_select (C, NULL, NULL, GxB_NONZERO, A, NULL, NULL)) ;

    // return C to MATLAB as a regular MATLAB sparse matrix
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C nonzero", false) ;

    FREE_ALL ;
}

