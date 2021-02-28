//------------------------------------------------------------------------------
// gb_mxstring_to_semiring: get a GraphBLAS semiring from a MATLAB string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_Semiring gb_mxstring_to_semiring    // return semiring from a string
(
    const mxArray *mxstring,            // MATLAB string
    const GrB_Type default_type         // default type if not in the string
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (gb_mxarray_is_empty (mxstring))
    { 
        ERROR ("semiring missing") ;
    }

    //--------------------------------------------------------------------------
    // get the string
    //--------------------------------------------------------------------------

    #define LEN 256
    char semiring_string [LEN+2] ;
    gb_mxstring_to_string (semiring_string, LEN, mxstring, "semiring") ;

    //--------------------------------------------------------------------------
    // convert the string to a semiring
    //--------------------------------------------------------------------------

    return (gb_string_to_semiring (semiring_string, default_type)) ;
}

