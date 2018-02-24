//------------------------------------------------------------------------------
// GB_Vector_check: print a GraphBLAS GrB_Vector and check if it is valid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_Vector: same as GrB_Matrix, except it has exactly one column

#include "GB.h"

GrB_Info GB_Vector_check    // check a GraphBLAS vector
(
    const GrB_Vector v,     // GraphBLAS vector to print and check
    const char *name,       // name of the vector
    const GB_diagnostic pr  // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
)
{

    //--------------------------------------------------------------------------
    // check GrB_Matrix conditions
    //--------------------------------------------------------------------------

    GrB_Info info = GB_object_check ((GrB_Matrix) v, name, pr, "vector") ;
    if (! (info == GrB_INDEX_OUT_OF_BOUNDS || info == GrB_SUCCESS))
    {
        // GrB_Matrix form is invalid already
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // check GrB_Vector specific conditions
    //--------------------------------------------------------------------------

    if (v->ncols > 1)
    {
        if (pr > 0) printf (
            "GrB_Vector has more than one column [%s], ncols = "GBd"\n",
            name, v->ncols) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "GrB_Vector has more than one column [%s], ncols = "GBd"",
            name, v->ncols))) ;
    }

    return (info) ; // pass info directly from GB_object_check (jumbled case)
}

