//------------------------------------------------------------------------------
// GB_mx_abort: terminate MATLAB
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_abort ( )                    // assertion failure
{
    mexErrMsgIdAndTxt ("GraphBLAS:assert", "assertion failed") ;
}

