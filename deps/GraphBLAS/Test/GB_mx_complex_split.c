//------------------------------------------------------------------------------
// GB_mx_complex_split: split a MATLAB complex mxArray
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_complex_split    // split complex array to real/imag part for MATLAB
(
    int64_t n,
    // input:
    const double *X,    // size 2*n, real and imaginary parts interleaved
    // output:
    mxArray *Y          // MATLAB array with n elements
)
{

    double *Yx = mxGetPr (Y) ;      // real array of size n
    double *Yz = mxGetPi (Y) ;      // imag array of size n

    for (int64_t k = 0 ; k < n ; k++)
    {
        Yx [k] = X [2*k  ] ;
        Yz [k] = X [2*k+1] ;
    }
}

