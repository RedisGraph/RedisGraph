//------------------------------------------------------------------------------
// GB_mex_qsort_3: sort using GB_qsort_3
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "[I,J,K] = qsort (I,J,K)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargin != 3 || nargout != 3)
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

    pargout [0] = mxCreateNumericMatrix (n, 1, mxINT64_CLASS, mxREAL) ;
    int64_t *Iout = mxGetData (pargout [0]) ;
    memcpy (Iout, I, n * sizeof (int64_t)) ;

    pargout [1] = mxCreateNumericMatrix (n, 1, mxINT64_CLASS, mxREAL) ;
    int64_t *Jout = mxGetData (pargout [1]) ;
    memcpy (Jout, J, n * sizeof (int64_t)) ;

    pargout [2] = mxCreateNumericMatrix (n, 1, mxINT64_CLASS, mxREAL) ;
    int64_t *Kout = mxGetData (pargout [2]) ;
    memcpy (Kout, K, n * sizeof (int64_t)) ;

    TIC ;

    GB_qsort_3 (Iout, Jout, Kout, n) ;

    TOC ;
    GB_mx_put_time (0) ;
}

