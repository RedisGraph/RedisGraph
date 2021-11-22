//------------------------------------------------------------------------------
// GrB_Matrix_wait: wait for a matrix to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Finishes all work on a matrix, followed by an OpenMP flush.

#include "GB.h"

#define GB_FREE_ALL ;

GrB_Info GrB_Matrix_wait    // finish all work on a matrix
(
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GrB_Matrix *A
    #else
    GrB_Matrix A,
    GrB_WaitMode waitmode
    #endif
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GB_WHERE ((*A), "GrB_Matrix_wait (&A)") ;
    GB_RETURN_IF_NULL (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*A) ;
    #else
    GB_WHERE (A, "GrB_Matrix_wait (A, waitmode)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    #endif

    //--------------------------------------------------------------------------
    // finish all pending work on the matrix
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    if (GB_ANY_PENDING_WORK (*A))
    {
        GrB_Info info ;
        GB_BURBLE_START ("GrB_Matrix_wait") ;
        GB_OK (GB_wait (*A, "matrix", Context)) ;
        GB_BURBLE_END ;
    }
    #else
    if (waitmode != GrB_COMPLETE && GB_ANY_PENDING_WORK (A))
    {
        GrB_Info info ;
        GB_BURBLE_START ("GrB_Matrix_wait") ;
        GB_OK (GB_wait (A, "matrix", Context)) ;
        GB_BURBLE_END ;
    }
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

