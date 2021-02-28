//------------------------------------------------------------------------------
// GB_mx_complex_merge: merge a MATLAB complex mxArray
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_complex_merge
(
    int64_t n,
    // output:
    double *X,          // size 2*n, real and imaginary parts interleaved
    // input:
    const mxArray *Y    // MATLAB array with n elements
)
{

    double *Yx = mxGetPr (Y) ;      // real array of size n
    double *Yz = mxGetPi (Y) ;      // imag array of size n

    for (int64_t k = 0 ; k < n ; k++)
    {
        X [2*k  ] = Yx [k] ;
        X [2*k+1] = Yz [k] ;
    }
}

