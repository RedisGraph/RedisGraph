//------------------------------------------------------------------------------
// GB_mex_dump: copy and print a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    Complex_finalize ( ) ;              \
    GB_mx_put_global (malloc_debug) ;   \
}

#include "GB_mex.h"

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

    // check inputs
    if (nargout > 1 || nargin < 1 || nargin > 2)
    {
        mexErrMsgTxt ("Usage: C = GB_mex_dump (A,pr)") ;
    }

    // get A (deep copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get pr
    GET_SCALAR (1, GB_diagnostic, pr, 1) ;

    // dump the matrix
    GrB_Info info = GB_check (A, "", pr) ;
    if (info != GrB_SUCCESS)
    {
        mexErrMsgTxt (GrB_error ( )) ;
    }

    // return A to MATLAB as a struct and free the GraphBLAS A
    pargout [0] = GB_mx_Matrix_to_mxArray (&A, "C output", true) ;

    FREE_ALL ;
}

