//------------------------------------------------------------------------------
// GrB_Vector_wait: wait for a vector to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Finishes all work on a vector, followed by an OpenMP flush.

#include "GB.h"

#define GB_FREE_ALL ;

GrB_Info GrB_Vector_wait    // finish all work on a vector
(
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GrB_Vector *v
    #else
    GrB_Vector v,
    GrB_WaitMode waitmode
    #endif
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GB_WHERE ((*v), "GrB_Vector_wait (&v)") ;
    GB_RETURN_IF_NULL (v) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*v) ;
    #else
    GB_WHERE (v, "GrB_Vector_wait (v, waitmode)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;
    #endif

    //--------------------------------------------------------------------------
    // finish all pending work on the vector
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    if (GB_ANY_PENDING_WORK (*v))
    {
        GrB_Info info ;
        GB_BURBLE_START ("GrB_Vector_wait") ;
        GB_OK (GB_wait ((GrB_Matrix) (*v), "vector", Context)) ;
        GB_BURBLE_END ;
    }
    #else
    if (waitmode != GrB_COMPLETE && GB_ANY_PENDING_WORK (v))
    {
        GrB_Info info ;
        GB_BURBLE_START ("GrB_Vector_wait") ;
        GB_OK (GB_wait ((GrB_Matrix) v, "vector", Context)) ;
        GB_BURBLE_END ;
    }
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

