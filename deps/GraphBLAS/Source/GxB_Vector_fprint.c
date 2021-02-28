//------------------------------------------------------------------------------
// GxB_Vector_fprint: print and check a GrB_Vector object
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Vector_fprint          // print and check a GrB_Vector
(
    GrB_Vector v,                   // object to print and check
    const char *name,               // name of the object
    GxB_Print_Level pr,             // print level
    FILE *f                         // file for output
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Vector_fprint (v, name, pr, f)") ;

    //--------------------------------------------------------------------------
    // print and check the object
    //--------------------------------------------------------------------------

    GrB_Info info = GB_Vector_check (v, name, pr, f, Context) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    if (info == GrB_INDEX_OUT_OF_BOUNDS)
    { 
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "vector invalid: indices out of order [%s]", GB_NAME))) ;
    }
    else
    { 
        return (info) ;
    }
}

