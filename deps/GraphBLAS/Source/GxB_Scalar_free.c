//------------------------------------------------------------------------------
// GxB_Scalar_free: free a sparse GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// free all the content of a GxB_Scalar.  After GxB_Scalar_free (&s), s is set
// to NULL

#include "GB.h"

GrB_Info GxB_Scalar_free    // free a GxB_Scalar
(
    GxB_Scalar *s           // handle of GxB_Scalar to free
)
{ 

    GB_SCALAR_FREE (s) ;
    return (GrB_SUCCESS) ;
}

