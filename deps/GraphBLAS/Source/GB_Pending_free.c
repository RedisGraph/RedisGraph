//------------------------------------------------------------------------------
// GB_Pending_free: free a list of pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_Pending.h"

void GB_Pending_free        // free a list of pending tuples
(
    GB_Pending *PHandle
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (PHandle != NULL) ;

    //--------------------------------------------------------------------------
    // free all pending tuples
    //--------------------------------------------------------------------------

    GB_Pending Pending = (*PHandle) ;
    if (Pending != NULL)
    { 
        GB_FREE (Pending->i) ;
        GB_FREE (Pending->j) ;
        GB_FREE (Pending->x) ;
        GB_FREE (Pending) ;
    }

    (*PHandle) = NULL ;
}

