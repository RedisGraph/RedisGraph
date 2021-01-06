//------------------------------------------------------------------------------
// GB_Scalar_check: print a GraphBLAS GxB_Scalar and check if it is valid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GxB_Scalar: same as a GrB_Vector of length 1

#include "GB.h"

GrB_Info GB_Scalar_check    // check a GraphBLAS GxB_Scalar
(
    const GxB_Scalar s,     // GraphBLAS GxB_Scalar to print and check
    const char *name,       // name of the GxB_Scalar
    int pr,                 // print level
    FILE *f                 // file for output
)
{

    //--------------------------------------------------------------------------
    // check GrB_Matrix conditions
    //--------------------------------------------------------------------------

    GrB_Info info = GB_matvec_check ((GrB_Matrix) s, name, pr, f, "scalar") ;
    if (info != GrB_SUCCESS)
    { 
        // GrB_Matrix form is invalid already
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // check GxB_Scalar specific conditions
    //--------------------------------------------------------------------------

    if (!GB_SCALAR_OK (s))
    { 
        GBPR0 ("    GxB_Scalar is invalid [%s]\n", name) ;
        return (GrB_INVALID_OBJECT) ;
    }

    return (GrB_SUCCESS) ;
}

