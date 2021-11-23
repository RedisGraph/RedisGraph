//------------------------------------------------------------------------------
// GrB_Scalar_wait: wait for a scalar to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Finishes all work on a scalar, followed by an OpenMP flush.

#include "GB.h"

#define GB_FREE_ALL ;

GrB_Info GrB_Scalar_wait    // finish all work on a scalar
(
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GrB_Scalar *s
    #else
    GrB_Scalar s,
    GrB_WaitMode waitmode
    #endif
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GB_WHERE ((*s), "GrB_Scalar_wait (&s)") ;
    GB_RETURN_IF_NULL (s) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*s) ;
    #else
    GB_WHERE (s, "GrB_Scalar_wait (s, waitmode)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (s) ;
    #endif

    //--------------------------------------------------------------------------
    // finish all pending work on the scalar
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    if (GB_ANY_PENDING_WORK (*s))
    {
        GrB_Info info ;
        GB_BURBLE_START ("GrB_Scalar_wait") ;
        GB_OK (GB_wait ((GrB_Matrix) (*s), "scalar", Context)) ;
        GB_BURBLE_END ;
    }
    #else
    if (waitmode != GrB_COMPLETE && GB_ANY_PENDING_WORK (s))
    {
        GrB_Info info ;
        GB_BURBLE_START ("GrB_Scalar_wait") ;
        GB_OK (GB_wait ((GrB_Matrix) s, "scalar", Context)) ;
        GB_BURBLE_END ;
    }
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GxB_Scalar_wait: wait for a scalar to complete (historical)
//------------------------------------------------------------------------------

GrB_Info GxB_Scalar_wait    // finish all work on a scalar
(
    GrB_Scalar *s
)
{
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    return (GrB_Scalar_wait (s)) ;
    #else
    return (GrB_Scalar_wait (*s, GrB_MATERIALIZE)) ;
    #endif
}

