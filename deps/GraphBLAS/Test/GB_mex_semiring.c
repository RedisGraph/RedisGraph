//------------------------------------------------------------------------------
// GB_mex_semiring: parse a semiring, for testing; returns nothing
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "GB_mex_semiring (semiring_struct))"

#define FREE_ALL            \
{                           \
    GB_mx_put_global (true, 0) ;        \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Semiring semiring = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargin != 1)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    GB_mx_mxArray_to_Semiring (&semiring, pargin [0], "semiring",
        mxDOUBLE_CLASS) ;

    GrB_Info info = GB_Semiring_check (semiring, "semiring", GxB_COMPLETE,
        NULL, NULL) ;
    if (info != GrB_SUCCESS)
    {
        mexErrMsgTxt (GrB_error ( )) ;
    }
    FREE_ALL ;
}

