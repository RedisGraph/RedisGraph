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

    ASSERT (A != NULL) ;

    //--------------------------------------------------------------------------
    // wait if mode is blocking, or if too many pending tuples
    //--------------------------------------------------------------------------

    if (!GB_ANY_PENDING_WORK (A))
    { 
        // no pending work, so no need to block
        return (GrB_SUCCESS) ;
    }

    double npending = (double) GB_Pending_n (A) ;
    double anzmax = ((double) A->vlen) * ((double) A->vdim) ;
    bool many_pending = (npending >= anzmax) ;
    bool blocking = (GB_Global_mode_get ( ) == GrB_BLOCKING) ;

    if (many_pending || blocking)
    { 
        // delete any lingering zombies and assemble any pending tuples
        GB_MATRIX_WAIT (A) ;
    }
    return (GrB_SUCCESS) ;
}

