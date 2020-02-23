//------------------------------------------------------------------------------
// GB_mex_dump: copy and print a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "hack = GB_mex_hack (hack)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    int64_t hack ;

    if (nargin > 1 || nargout > 1)
    {
        mexErrMsgTxt ("usage: " USAGE "\n") ;
    }

    if (nargin == 0)
    {
        hack = GB_Global_hack_get ( ) ;
    }
    else
    {
        double *p = mxGetPr (pargin [0]) ;
        hack = (int64_t) p [0] ;
        GB_Global_hack_set (hack) ;
    }

    pargout [0] = mxCreateDoubleScalar ((double) hack) ;
}

