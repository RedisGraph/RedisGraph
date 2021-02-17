//------------------------------------------------------------------------------
// GxB_Scalar_free: free a sparse GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// free all the content of a GxB_Scalar.  After GxB_Scalar_free (&s), s is set
// to NULL

#include "GB.h"

GrB_Info GxB_Scalar_free    // free a GxB_Scalar
(
    GxB_Scalar *s           // handle of GxB_Scalar to free
)
{ 

    GB_Matrix_free ((GrB_Matrix *) s) ;
    return (GrB_SUCCESS) ;
}

