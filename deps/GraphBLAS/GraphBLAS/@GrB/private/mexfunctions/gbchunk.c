//------------------------------------------------------------------------------
// gbchunk: get/set the chunk size to use in GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin <= 1 && nargout <= 1,
        "usage: c = GrB.chunk ; or GrB.chunk (c)") ;

    //--------------------------------------------------------------------------
    // set the chunk, if requested
    //--------------------------------------------------------------------------

    double c ;

    if (nargin > 0)
    { 
        // set the chunk
        CHECK_ERROR (!gb_mxarray_is_scalar (pargin [0]),
            "input must be a scalar") ;
        c = (double) mxGetScalar (pargin [0]) ;
        OK (GxB_Global_Option_set (GxB_CHUNK, c)) ;
    }

    //--------------------------------------------------------------------------
    // return the chunk
    //--------------------------------------------------------------------------

    OK (GxB_Global_Option_get (GxB_CHUNK, &c)) ;
    pargout [0] = mxCreateDoubleScalar (c) ;
    GB_WRAPUP ;
}

