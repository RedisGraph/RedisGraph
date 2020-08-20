//------------------------------------------------------------------------------
// gbnvals: number of entries in a GraphBLAS matrix struct
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input may be either a GraphBLAS matrix struct or a standard MATLAB
// sparse matrix.

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

    gb_usage (nargin == 1 && nargout <= 1, "usage: nvals = GrB.nvals (X)") ;

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

