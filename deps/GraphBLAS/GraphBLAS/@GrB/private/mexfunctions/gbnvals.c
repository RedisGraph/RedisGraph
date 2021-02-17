//------------------------------------------------------------------------------
// gbnvals: number of entries in a GraphBLAS matrix struct
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The input may be either a GraphBLAS matrix struct or a standard MATLAB
// sparse matrix.

// Usage

// nvals = gbnvals (X)

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

    gb_usage (nargin == 1 && nargout <= 1, "usage: nvals = gbnvals (X)") ;

    //--------------------------------------------------------------------------
    // get the # of entries in the matrix
    //--------------------------------------------------------------------------

    GrB_Matrix X = gb_get_shallow (pargin [0]) ;
    GrB_Index nvals ;
    OK (GrB_Matrix_nvals (&nvals, X)) ;
    pargout [0] = mxCreateDoubleScalar ((double) nvals) ;
    OK (GrB_Matrix_free (&X)) ;
    GB_WRAPUP ;
}

