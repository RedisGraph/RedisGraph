//------------------------------------------------------------------------------
// GxB_Scalar_dup: make a deep copy of a sparse GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// s = t, making a deep copy

#include "GB.h"

GrB_Info GxB_Scalar_dup     // make an exact copy of a GxB_Scalar
(
    GxB_Scalar *s,          // handle of output GxB_Scalar to create
    const GxB_Scalar t      // input GxB_Scalar to copy
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Scalar_dup (&s, t)") ;
    GB_RETURN_IF_NULL (s) ;
    GB_RETURN_IF_NULL_OR_FAULTY (t) ;
    ASSERT (GB_SCALAR_OK (t)) ;

    //--------------------------------------------------------------------------
    // duplicate the GxB_Scalar
    //--------------------------------------------------------------------------

    return (GB_dup ((GrB_Matrix *) s, (GrB_Matrix) t, Context)) ;
}

