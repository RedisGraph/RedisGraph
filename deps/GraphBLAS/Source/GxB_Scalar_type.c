//------------------------------------------------------------------------------
// GxB_Scalar_type: return the type of a GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Scalar_type    // get the type of a GxB_Scalar
(
    GrB_Type *type,         // returns the type of the GxB_Scalar
    const GxB_Scalar s      // GxB_Scalar to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Scalar_type (&type, s)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (s) ;

    //--------------------------------------------------------------------------
    // get the type
    //--------------------------------------------------------------------------

    return (GB_matvec_type (type, (GrB_Matrix) s, Context)) ;
}

