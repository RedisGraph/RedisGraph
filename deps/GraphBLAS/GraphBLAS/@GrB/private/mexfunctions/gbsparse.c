//------------------------------------------------------------------------------
// gbsparse: convert a GraphBLAS matrix struct into a MATLAB sparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input may be either a GraphBLAS matrix struct or a standard MATLAB
// sparse matrix.  The output is a standard MATLAB sparse matrix.

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

    gb_usage (nargin == 2 && nargout <= 1, "usage: A = GrB.sparse (X, type)") ;

    //--------------------------------------------------------------------------
    // get the input matrix
    //--------------------------------------------------------------------------

    GrB_Matrix X = gb_get_shallow (pargin [0]) ;
    GrB_Type xtype ;
    OK (GxB_Matrix_type (&xtype, X)) ;

    //--------------------------------------------------------------------------
    // get the desired type, and typecast if needed
    //--------------------------------------------------------------------------

    GrB_Type type = gb_mxstring_to_type (pargin [1]) ;
    if (type != xtype)
    { 
        GrB_Matrix T = gb_typecast (type, GxB_BY_COL, X) ;
        OK (GrB_Matrix_free (&X)) ;
        X = T ;
    }

    //--------------------------------------------------------------------------
    // export the input matrix to a MATLAB sparse matrix
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&X, KIND_SPARSE) ;
    GB_WRAPUP ;
}

