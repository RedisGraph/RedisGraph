//------------------------------------------------------------------------------
// GB_mex_isequal: returns true if A and B are equal
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "C = GB_mex_isequal (A, B)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&B) ;               \
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
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;

    GB_WHERE (USAGE) ;

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

    GrB_BinaryOp op = NULL ;
    if (mxIsComplex (pargin [0]))
    {
        op = Complex_eq ;
    }

    // C = all (A == B) using the op
    bool result ;
    METHOD (isequal (&result, A, B, op)) ;

    // return C to MATLAB as a plain sparse matrix
    pargout [0] = mxCreateDoubleScalar ((double) result) ;

    FREE_ALL ;
}

