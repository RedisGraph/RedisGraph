//------------------------------------------------------------------------------
// GxB_Scalar_wait: wait for a scalar to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Finishes all work on a scalar, followed by an OpenMP flush.

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

    #pragma omp flush
    GB_WHERE ((*s), "GxB_Scalar_wait (&s)") ;
    GB_RETURN_IF_NULL (s) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*s) ;

    //--------------------------------------------------------------------------
    // finish all pending work on the scalar
    //--------------------------------------------------------------------------

    if (GB_ANY_PENDING_WORK (*s))
    { 
        GrB_Info info ;
        GB_BURBLE_START ("GxB_Scalar_wait") ;
        GB_OK (GB_wait ((GrB_Matrix) (*s), "scalar", Context)) ;
        GB_BURBLE_END ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

