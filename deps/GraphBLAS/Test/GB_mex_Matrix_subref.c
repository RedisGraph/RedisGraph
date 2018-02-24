//------------------------------------------------------------------------------
// GB_mex_Matrix_subref: C=A(I,J) or C=A(J,I)'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

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
    GrB_Matrix A = NULL ;
    GrB_Matrix C = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 3 || nargin > 4)
    {
        mexErrMsgTxt ("Usage: C = GB_mex_Matrix_subref (A, I, J, atrans)") ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get I
    GrB_Index *I, ni ; 
    if (!GB_mx_mxArray_to_indices (&I, pargin [1], &ni))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I failed") ;
    }

    // get J
    GrB_Index *J, nj ; 
    if (!GB_mx_mxArray_to_indices (&J, pargin [2], &nj))
    {
        FREE_ALL ;
        mexErrMsgTxt ("J failed") ;
    }

    // get the atranspose option
    GET_SCALAR (3, bool, atranspose, false) ;

    // C = A(I,J) or A(J,I)', no need to check dimensions of C
    METHOD (GB_subref_numeric (&C, 0, 0, A, I, ni, J, nj, atranspose)) ;

    // return C to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C subref result", false) ;

    FREE_ALL ;
}

