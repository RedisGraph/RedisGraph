//------------------------------------------------------------------------------
// GB_mex_ipagerank: compute pagerank with an integer semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This is for testing only.

#include "GB_mex.h"
#include "graphblas_demos.h"

#define USAGE "[r,irank] = GB_mex_ipagerank (A)"

#define FREE_ALL                        \
{                                       \
    if (P != NULL) mxFree (P) ;         \
    GrB_Matrix_free_(&A) ;               \
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

    GrB_Info info = GrB_SUCCESS ;
    GrB_Matrix A = NULL ;
    iPageRank *P = NULL ;
    GrB_Index n = 0 ;
    bool malloc_debug = GB_mx_get_global (true) ;

    // check inputs
    if (nargout > 2 || nargin != 1)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    GrB_Matrix_nrows (&n, A) ;

    // compute the iPageRank P
    GB_MEX_TIC ;
    ipagerank (&P, A) ;
    GB_MEX_TOC ;

    // return iPageRank to MATLAB
    pargout [0] = mxCreateDoubleMatrix (1, n, mxREAL) ;
    pargout [1] = mxCreateDoubleMatrix (1, n, mxREAL) ;

    double *r     = mxGetPr (pargout [0]) ;
    double *irank = mxGetPr (pargout [1]) ;

    // add one to the page ID to convert 0-based to 1-based
    for (int64_t i = 0 ; i < n ; i++)
    {
        r     [i] = P [i].pagerank ;
        irank [i] = P [i].page + 1 ;
    }

    FREE_ALL ;
}

