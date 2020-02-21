//------------------------------------------------------------------------------
// gbsemiringinfo: print a GraphBLAS semiring (for illustration only)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Usage:

// gbsemiringinfo (semiring_string)
// gbsemiringinfo (semiring_string, type)

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
    "usage: GrB.semiringinfo (semiring) or GrB.semiringinfo (semiring,type)") ;

    //--------------------------------------------------------------------------
    // construct the GraphBLAS semiring and print it
    //--------------------------------------------------------------------------

    #define LEN 256
    char opstring [LEN+2] ;
    gb_mxstring_to_string (opstring, LEN, pargin [0], "binary operator") ;

    GrB_Type type = NULL ;
    if (nargin == 2)
    { 
        type = gb_mxstring_to_type (pargin [1]) ;
        CHECK_ERROR (type == NULL, "unknown type") ;
    }

    GrB_Semiring semiring = gb_mxstring_to_semiring (pargin [0], type) ;
    OK (GxB_Semiring_fprint (semiring, opstring, GxB_COMPLETE, NULL)) ;
    GB_WRAPUP ;
}

