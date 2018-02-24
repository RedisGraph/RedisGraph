//------------------------------------------------------------------------------
// GB_mex_semiring: parse a semiring, for testing; returns nothing
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Usage:
// GB_mex_semiring (semiring_struct)

#include "GB_mex.h"

#define FREE_ALL            \
{                           \
    GB_mx_put_global (malloc_debug) ; \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global ( ) ;
    GrB_Semiring semiring = NULL ;

    // check inputs
    if (nargin != 1)
    {
        mexErrMsgTxt ("Usage: GB_mex_semiring (semiring_struct))") ;
    }

    GB_mx_mxArray_to_Semiring (&semiring, pargin [0], "semiring",
        mxDOUBLE_CLASS) ;

    GrB_Info info = GB_check (semiring, "semiring", 3) ;
    if (info != GrB_SUCCESS)
    {
        mexErrMsgTxt (GrB_error ( )) ;
    }
    FREE_ALL ;
}

