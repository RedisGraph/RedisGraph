//------------------------------------------------------------------------------
// GB_mex_dump: copy and print a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "C = GB_mex_dump (A,pr)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    Complex_finalize ( ) ;              \
    GB_mx_put_global (false, 0) ;       \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global (false) ;
    GrB_Matrix A = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 1 || nargin > 2)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A (deep copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", true, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get pr
    int GET_SCALAR (1, int, pr, 1) ;

    // dump the matrix
    GrB_Info info = GB_Matrix_check (A, "", pr, stdout, NULL) ;
    if (info != GrB_SUCCESS)
    {
        mexErrMsgTxt (GrB_error ( )) ;
    }

    // return A to MATLAB as a struct and free the GraphBLAS A
    pargout [0] = GB_mx_Matrix_to_mxArray (&A, "C output", true) ;

    FREE_ALL ;
}

