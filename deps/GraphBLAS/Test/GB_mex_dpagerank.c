//------------------------------------------------------------------------------
// GB_mex_dpagerank: compute pagerank with a real semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is for testing only.

#include "GB_mex.h"
#include "demos.h"

#define USAGE "[r,irank,iters] = GB_mex_dpagerank (A, method)"

#define FREE_ALL                        \
{                                       \
    if (P != NULL) free (P) ;           \
    GB_MATRIX_FREE (&A) ;               \
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

    GrB_Info info = GrB_SUCCESS ;
    GrB_Matrix A = NULL ;
    PageRank *P = NULL ;
    bool malloc_debug = GB_mx_get_global (true) ;

    GB_WHERE (USAGE) ;

    // check inputs
    if (nargout > 3 || nargin < 1 || nargin > 2)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get the method 
    int GET_SCALAR (1, int, method, GxB_DEFAULT) ;

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    GrB_Index n ;
    GrB_Matrix_nrows (&n, A) ;

    // compute the PageRank P
    int iters = 0 ;
    TIC ;
    if (nargin > 1)
    {
        printf ("dpagerank2, method %d\n", method) ;
        info = dpagerank2 (&P, A, 100, 1e-5, &iters, method) ;
    }
    else // default method
    {
        info= dpagerank (&P, A) ;
    }
    TOC ;

    if (info != GrB_SUCCESS)
    {
        FREE_ALL ;
        printf ("%s\n", GrB_error ( )) ;
        mexErrMsgTxt ("failed") ;
    }

    // return PageRank to MATLAB
    pargout [0] = mxCreateDoubleMatrix (1, n, mxREAL) ;
    pargout [1] = mxCreateDoubleMatrix (1, n, mxREAL) ;
    pargout [2] = mxCreateDoubleScalar ((double) iters) ;

    double *r     = mxGetPr (pargout [0]) ;
    double *irank = mxGetPr (pargout [1]) ;

    // add one to the page ID to convert 0-based to 1-based
    for (int64_t i = 0 ; i < n ; i++)
    {
        r     [i] = P [i].pagerank ;
        irank [i] = P [i].page + 1 ;
    }

    FREE_ALL ;
    GrB_finalize ( ) ;
}

