//------------------------------------------------------------------------------
// GrB_IndexUnaryOp_wait: wait for a user-defined GrB_IndexUnaryOp to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// In SuiteSparse:GraphBLAS, a user-defined GrB_IndexUnaryOp has no pending
// operations to wait for.  All this method does is verify that the op is
// properly initialized, and then it does an OpenMP flush.

#include "GB.h"

GrB_Info GrB_IndexUnaryOp_wait   // no work, just check if valid
(
    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GrB_IndexUnaryOp *op
    #else
    GrB_IndexUnaryOp op,
    GrB_WaitMode waitmode
    #endif
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #if (GxB_IMPLEMENTATION_MAJOR <= 5)
    GB_WHERE1 ("GrB_IndexUnaryOp_wait (&op)") ;
    GB_RETURN_IF_NULL (op) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*op) ;
    #else
    GB_WHERE1 ("GrB_IndexUnaryOp_wait (op, waitmode)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    #pragma omp flush
    return (GrB_SUCCESS) ;
}

