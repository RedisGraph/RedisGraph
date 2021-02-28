//------------------------------------------------------------------------------
// GxB_Semiring_fprint: print and check a GrB_Semiring object
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Semiring_fprint        // print and check a GrB_Semiring
(
    GrB_Semiring semiring,          // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Semiring_fprint (semiring, name, pr, f)") ;

    //--------------------------------------------------------------------------
    // print and check the object
    //--------------------------------------------------------------------------

    return (GB_Semiring_check (semiring, name, pr, f, Context)) ;
}

