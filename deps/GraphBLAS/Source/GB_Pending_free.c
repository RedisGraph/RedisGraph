//------------------------------------------------------------------------------
// GB_Pending_free: free a list of pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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
        GB_FREE_MEMORY (Pending->i, Pending->nmax, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Pending->j, Pending->nmax, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Pending->x, Pending->nmax, Pending->size) ;
        GB_FREE_MEMORY (Pending, 1, sizeof (struct GB_Pending_struct)) ;
    }

    (*PHandle) = NULL ;
}

