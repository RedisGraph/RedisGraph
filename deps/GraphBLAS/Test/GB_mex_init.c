//------------------------------------------------------------------------------
// GB_mex_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "GB_mex_init"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    GB_WHERE (USAGE) ;
    GB_Global_user_multithreaded_set (false) ;
    GxB_init (GrB_NONBLOCKING, mxMalloc, mxCalloc, mxRealloc, mxFree) ;
    GB_Global_abort_function_set (GB_mx_abort) ;
    GB_Global_malloc_tracking_set (true) ;
    GxB_set (GxB_FORMAT, GxB_BY_COL) ;
}

