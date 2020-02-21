//------------------------------------------------------------------------------
// GB_mex_qsort: sort int64's using GB_qsort_1a
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "J = qsort (I)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    // check inputs
    if (nargin != 1)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    if (!mxIsClass (pargin [0], "int64"))
    {
        mexErrMsgTxt ("I must be a int64 array") ;
    }
    int64_t *I = mxGetData (pargin [0]) ;
    int64_t n = (uint64_t) mxGetNumberOfElements (pargin [0]) ;

    pargout [0] = mxCreateNumericMatrix (1, n, mxINT64_CLASS, mxREAL) ;
    int64_t *J = mxGetData (pargout [0]) ;
    memcpy (J, I, n * sizeof (int64_t)) ;

    GB_MEX_TIC ;

    GB_qsort_1a (J, n) ;

    GB_MEX_TOC ;

    GB_mx_put_time (0) ;
}

