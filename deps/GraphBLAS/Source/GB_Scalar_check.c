//------------------------------------------------------------------------------
// GB_Scalar_check: print a GraphBLAS GxB_Scalar and check if it is valid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GxB_Scalar: same as a GrB_Vector of length 1

#include "GB.h"

GrB_Info GB_Scalar_check    // check a GraphBLAS GxB_Scalar
(
    const GxB_Scalar s,     // GraphBLAS GxB_Scalar to print and check
    const char *name,       // name of the GxB_Scalar
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check GrB_Matrix conditions
    //--------------------------------------------------------------------------

    GrB_Info info = GB_matvec_check ((GrB_Matrix) s, name, pr, f, "scalar",
        Context) ;
    if (! (info == GrB_INDEX_OUT_OF_BOUNDS || info == GrB_SUCCESS))
    { 
        // GrB_Matrix form is invalid already
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // check GxB_Scalar specific conditions
    //--------------------------------------------------------------------------

    if (!GB_SCALAR_OK (s))
    { 
        GBPR0 ("GxB_Scalar is invalid [%s]\n", name) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "GxB_Scalar is invalid [%s]", name))) ;
    }

    return (info) ; // pass info directly from GB_matvec_check (jumbled case)
}

