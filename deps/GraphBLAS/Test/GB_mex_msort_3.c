//------------------------------------------------------------------------------
// GB_mex_msort_3: sort using GB_msort_3
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "[I,J,K] = GB_mex_msort_3 (I,J,K,nthreads)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    bool malloc_debug = GB_mx_get_global (true) ;

    // check inputs
    if (nargin != 4 || nargout != 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }
    if (!mxIsClass (pargin [0], "int64"))
    {
        mexErrMsgTxt ("I must be a int64 array") ;
    }
    if (!mxIsClass (pargin [1], "int64"))
    {
        mexErrMsgTxt ("J must be a int64 array") ;
    }
    if (!mxIsClass (pargin [2], "int64"))
    {
        mexErrMsgTxt ("K must be a int64 array") ;
    }

    int64_t *I = mxGetData (pargin [0]) ;
    int64_t n = (uint64_t) mxGetNumberOfElements (pargin [0]) ;

    int64_t *J = mxGetData (pargin [1]) ;
    if (n != (uint64_t) mxGetNumberOfElements (pargin [1])) 
    {
        mexErrMsgTxt ("I and J must be the same length") ;
    }

    int64_t *K = mxGetData (pargin [2]) ;
    if (n != (uint64_t) mxGetNumberOfElements (pargin [2])) 
    {
        mexErrMsgTxt ("I and K must be the same length") ;
    }

    int GET_SCALAR (3, int, nthreads, 1) ;

    pargout [0] = mxCreateNumericMatrix (n, 1, mxINT64_CLASS, mxREAL) ;
    int64_t *Iout = mxGetData (pargout [0]) ;
    memcpy (Iout, I, n * sizeof (int64_t)) ;

    pargout [1] = mxCreateNumericMatrix (n, 1, mxINT64_CLASS, mxREAL) ;
    int64_t *Jout = mxGetData (pargout [1]) ;
    memcpy (Jout, J, n * sizeof (int64_t)) ;

    pargout [2] = mxCreateNumericMatrix (n, 1, mxINT64_CLASS, mxREAL) ;
    int64_t *Kout = mxGetData (pargout [2]) ;
    memcpy (Kout, K, n * sizeof (int64_t)) ;

    // get workspace
    int64_t *Work_0 = mxMalloc ((n+1) * sizeof (int64_t)) ;
    int64_t *Work_1 = mxMalloc ((n+1) * sizeof (int64_t)) ;
    int64_t *Work_2 = mxMalloc ((n+1) * sizeof (int64_t)) ;

    GB_MEX_TIC ;

    GB_msort_3 (Iout, Jout, Kout, Work_0, Work_1, Work_2, n, nthreads) ;

    GB_MEX_TOC ;

    // free workspace
    mxFree (Work_0) ;
    mxFree (Work_1) ;
    mxFree (Work_2) ;

    GB_mx_put_global (true, 0) ;
}

