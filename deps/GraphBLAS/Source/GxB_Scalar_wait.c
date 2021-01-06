//------------------------------------------------------------------------------
// GxB_Scalar_wait: wait for a scalar to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

#define GB_FREE_ALL ;

GrB_Info GxB_Scalar_wait    // finish all work on a scalar
(
    GxB_Scalar *s
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ((*s), "GxB_Scalar_wait (&s)") ;
    GB_BURBLE_START ("GxB_Scalar_wait") ;
    GB_RETURN_IF_NULL (s) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*s) ;

    //--------------------------------------------------------------------------
    // finish all pending work on the scalar
    //--------------------------------------------------------------------------

    GB_MATRIX_WAIT (*s) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    GB_BURBLE_END ;
    return (GrB_SUCCESS) ;
}

