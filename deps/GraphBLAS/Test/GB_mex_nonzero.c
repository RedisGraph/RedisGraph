//------------------------------------------------------------------------------
// GB_mex_nonzero: compute C=nonzero(A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = nonzero (A), where A and C are double

#include "GB_mex.h"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GB_mx_put_global (malloc_debug) ;   \
}


void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global ( ) ;
    GrB_Matrix A = NULL, C = NULL ;

    // check inputs
    if (nargout > 1 || nargin != 1)
    {
        mexErrMsgTxt ("Usage: C = GB_mex_nonzero (A)") ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    // construct C
    METHOD (GrB_Matrix_new (&C, GrB_FP64, A->nrows, A->ncols)) ;

    // C = nonzero (A)
    METHOD (GxB_Matrix_select (C, NULL, NULL, GxB_NONZERO, A, NULL, NULL)) ;

    // return C to MATLAB as a regular MATLAB sparse matrix
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C nonzero", false) ;

    FREE_ALL ;
}

