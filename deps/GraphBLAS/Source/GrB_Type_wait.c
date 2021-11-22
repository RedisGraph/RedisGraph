//------------------------------------------------------------------------------
// GrB_Type_wait: wait for a user-defined GrB_Type to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// In SuiteSparse:GraphBLAS, a user-defined GrB_Type has no pending operations
// to wait for.  All this method does is verify that the type is properly
// initialized, and then it does an OpenMP flush.

#include "GB.h"

GrB_Info GrB_Type_wait      // no work, just check if the GrB_Type is valid
(
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GrB_Type *type
    #else
    GrB_Type type,
    GrB_WaitMode waitmode
    #endif
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GB_WHERE1 ("GrB_Type_wait (&type)") ;
    GB_RETURN_IF_NULL (type) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*type) ;
    #else
    GB_WHERE1 ("GrB_Type_wait (type, waitmode)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (type) ;
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

