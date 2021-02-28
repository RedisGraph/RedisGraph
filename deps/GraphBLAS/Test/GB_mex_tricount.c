//------------------------------------------------------------------------------
// GB_mex_tricount: count the number of triangles in a graph
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

// Usage: ntri = GB_mex_tricount (method, A, E, L, U) ;
// see tricount.c for a description of the inputs

// Not all methods use all matrices.  If a matrix is not required, pass in
// sparse(1) for that parameter.  This is ugly, but this function is not meant
// to have a clean API.  It is simply for testing.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "[ntri t] = GB_mex_tricount (method, A, E, L, U)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&E) ;               \
    GB_MATRIX_FREE (&L) ;               \
    GB_MATRIX_FREE (&U) ;               \
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
    GrB_Matrix A = NULL, E = NULL, L = NULL, U = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 2 || nargin != 5)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get the method.  Default is Sandia method (outer product)
    int GET_SCALAR (0, int, method, 3) ;

    // get A, E, L, and U
    A = GB_mx_mxArray_to_Matrix (pargin [1], "A", false, true) ;
    E = GB_mx_mxArray_to_Matrix (pargin [2], "E", false, true) ;
    L = GB_mx_mxArray_to_Matrix (pargin [3], "L", false, true) ;
    U = GB_mx_mxArray_to_Matrix (pargin [4], "U", false, true) ;

    // count the triangles
    double t [2] ;
    int64_t ntri ;
    METHOD (tricount (&ntri, method, A, E, L, U, t)) ;

    // return ntri to MATLAB
    pargout [0] = mxCreateDoubleScalar ((double) ntri) ;

    // return t to MATLAB (compute time)
    if (nargout > 0)
    {
        pargout [1] = mxCreateDoubleScalar (t [0] + t [1]) ;
    }

    FREE_ALL ;
}

