//------------------------------------------------------------------------------
// GrB_Semiring_wait: wait for a user-defined GrB_Semiring to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// In SuiteSparse:GraphBLAS, a user-defined GrB_Semiring has no pending
// operations to wait for.  All this method does is verify that the semiring is
// properly initialized, and then it does an OpenMP flush.

#include "GB.h"

GrB_Info GrB_Semiring_wait   // no work, just check if the GrB_Semiring is valid
(
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GrB_Semiring *semiring
    #else
    GrB_Semiring semiring,
    GrB_WaitMode waitmode
    #endif
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GB_WHERE1 ("GrB_Semiring_wait (&semiring)") ;
    GB_RETURN_IF_NULL (semiring) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*semiring) ;
    #else
    GB_WHERE1 ("GrB_Semiring_wait (semiring, mode)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (semiring) ;
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

