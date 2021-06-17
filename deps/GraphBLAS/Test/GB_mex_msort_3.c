//------------------------------------------------------------------------------
// GB_mex_msort_3: sort using GB_msort_3b
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

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

    pargout [0] = GB_mx_create_full (n, 1, GrB_INT64) ;
    int64_t *Iout = mxGetData (pargout [0]) ;
    memcpy (Iout, I, n * sizeof (int64_t)) ;

    pargout [1] = GB_mx_create_full (n, 1, GrB_INT64) ;
    int64_t *Jout = mxGetData (pargout [1]) ;
    memcpy (Jout, J, n * sizeof (int64_t)) ;

    pargout [2] = GB_mx_create_full (n, 1, GrB_INT64) ;
    int64_t *Kout = mxGetData (pargout [2]) ;
    memcpy (Kout, K, n * sizeof (int64_t)) ;

    GB_MEX_TIC ;
    GB_msort_3b (Iout, Jout, Kout, n, nthreads) ;
    GB_MEX_TOC ;

    GB_mx_put_global (true) ;   
}

