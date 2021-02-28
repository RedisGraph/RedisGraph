//------------------------------------------------------------------------------
// gbunopinfo : print a GraphBLAS unary op (for illustration only)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Usage:

// gbunopinfo (unop)
// gbunopinfo (unop, type)

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

    gb_usage (nargin <= 2 && nargout == 0,
        "usage: GrB.unopinfo (unop) or GrB.unopinfo (unop,type)") ;

    //--------------------------------------------------------------------------
    // construct the GraphBLAS unary operator and print it
    //--------------------------------------------------------------------------

    #define LEN 256
    char opstring [LEN+2] ;
    gb_mxstring_to_string (opstring, LEN, pargin [0], "unary operator") ;

    GrB_Type type = NULL ;
    if (nargin == 2)
    { 
        type = gb_mxstring_to_type (pargin [1]) ;
        CHECK_ERROR (type == NULL, "unknown type") ;
    }

    GrB_UnaryOp op = gb_mxstring_to_unop (pargin [0], type) ;
    OK (GxB_UnaryOp_fprint (op, opstring, GxB_COMPLETE, NULL)) ;
    GB_WRAPUP ;
}

