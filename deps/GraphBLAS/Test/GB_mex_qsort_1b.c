//------------------------------------------------------------------------------
// GB_mex_qsort_1b: sort using GB_qsort_1b
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "[I,J] = qsort (I,J)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    // check inputs
    if (nargin != 2 || nargout != 2)
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

    int64_t *I = mxGetData (pargin [0]) ;
    int64_t n = (uint64_t) mxGetNumberOfElements (pargin [0]) ;

    int64_t *J = mxGetData (pargin [1]) ;
    if (n != (uint64_t) mxGetNumberOfElements (pargin [1])) 
    {
        mexErrMsgTxt ("I and J must be the same length") ;
    }

    pargout [0] = mxCreateNumericMatrix (n, 1, mxINT64_CLASS, mxREAL) ;
    int64_t *Iout = mxGetData (pargout [0]) ;
    memcpy (Iout, I, n * sizeof (int64_t)) ;

    pargout [1] = mxCreateNumericMatrix (n, 1, mxINT64_CLASS, mxREAL) ;
    int64_t *Jout = mxGetData (pargout [1]) ;
    memcpy (Jout, J, n * sizeof (int64_t)) ;

    GB_MEX_TIC ;

    GB_qsort_1b (Iout, Jout, sizeof (int64_t), n) ;

    GB_MEX_TOC ;
    GB_mx_put_time (0) ;
}

