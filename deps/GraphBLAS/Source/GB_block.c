//------------------------------------------------------------------------------
// GB_block: apply all pending computations if blocking mode enabled
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_Pending.h"

#define GB_FREE_ALL ;

GB_PUBLIC
GrB_Info GB_block   // apply all pending computations if blocking mode enabled
(
    GrB_Matrix A,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (A != NULL) ;

    //--------------------------------------------------------------------------
    // wait if mode is blocking, or if too many pending tuples
    //--------------------------------------------------------------------------

    if (!(GB_ANY_PENDING_WORK (A) || GB_NEED_HYPER_HASH (A)))
    { 
        // no pending work, so no need to block
        return (GrB_SUCCESS) ;
    }

    double npending = (double) GB_Pending_n (A) ;
    double anzmax = ((double) A->vlen) * ((double) A->vdim) ;
    bool many_pending = (npending >= anzmax) ;
    GrB_Mode mode = GB_Global_mode_get ( ) ;
    bool blocking = (mode == GrB_BLOCKING || mode == GxB_BLOCKING_GPU) ;

    if (many_pending || blocking)
    { 
        // delete any lingering zombies, assemble any pending tuples,
        // sort the vectors, and construct the A->Y hyper_hash
        GB_OK (GB_wait (A, "matrix", Context)) ;
        GB_OK (GB_hyper_hash_build (A, Context)) ;
    }
    return (GrB_SUCCESS) ;
}

