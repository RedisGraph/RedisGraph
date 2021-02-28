//------------------------------------------------------------------------------
// gb_mxstring_to_type: return the GraphBLAS type from a MATLAB string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

GrB_Type gb_mxstring_to_type    // return the GrB_Type from a MATLAB string
(
    const mxArray *mxstring
)
{

    #define LEN 256
    char s [LEN+2] ;
    gb_mxstring_to_string (s, LEN, mxstring, "type") ;
    return (gb_string_to_type (s)) ;
}

