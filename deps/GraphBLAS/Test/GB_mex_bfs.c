//------------------------------------------------------------------------------
// GB_mex_bfs: v = bfs (A,s)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "v = GB_mex_bfs (A,source)"

#define FREE_ALL                            \
{                                           \
    GB_MATRIX_FREE (&A) ;                   \
    GB_VECTOR_FREE (&v) ;                   \
    GB_mx_put_global (true, 0) ;            \
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
    GrB_Vector v = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 1 || nargin > 2)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, false) ;
    if (A == NULL && !mxIsEmpty (pargin [0]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get source (default is node 1)
    GrB_Index GET_SCALAR (1, GrB_Index, source, 0) ;

    // convert source to zero-based
    source-- ;
    printf ("zero-based source node now %g\n", (double) source) ;

    // do the bfs
    bfs5m (&v, A, source) ;

    // return v to MATLAB
    pargout [0] = GB_mx_Vector_to_mxArray (&v, "v output from bfs", false) ;

    FREE_ALL ;
}

