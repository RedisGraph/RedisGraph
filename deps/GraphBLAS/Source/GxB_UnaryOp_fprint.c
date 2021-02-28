//------------------------------------------------------------------------------
// GxB_UnaryOp_fprint: print and check a GrB_UnaryOp object
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_UnaryOp_fprint         // print and check a GrB_UnaryOp
(
    GrB_UnaryOp unaryop,            // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_UnaryOp_fprint (unaryop, name, pr, f)") ;

    //--------------------------------------------------------------------------
    // print and check the object
    //--------------------------------------------------------------------------

    return (GB_UnaryOp_check (unaryop, name, pr, f, Context)) ;
}

