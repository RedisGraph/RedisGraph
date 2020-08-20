//------------------------------------------------------------------------------
// gbisequal: isequal (A,B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbisequal returns isequal(A,B) for two matrices A and B.

// Usage:

//  result = gbisequal (A,B)

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

    gb_usage (nargin == 2 && nargout <= 1, "usage: s = GrB.isequal (A, B)") ;

    //--------------------------------------------------------------------------
    // get the arguments
    //--------------------------------------------------------------------------

    GrB_Matrix A = gb_get_shallow (pargin [0]) ;
    GrB_Matrix B = gb_get_shallow (pargin [1]) ;

    //--------------------------------------------------------------------------
    // check if they are equal
    //--------------------------------------------------------------------------

    pargout [0] = mxCreateLogicalScalar (gb_is_equal (A, B)) ;
    GB_WRAPUP ;
}

