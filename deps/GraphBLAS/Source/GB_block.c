//------------------------------------------------------------------------------
// GB_block: apply all pending computations if blocking mode enabled
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_block   // apply all pending computations if blocking mode enabled
(
    GrB_Matrix A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;

    //--------------------------------------------------------------------------
    // check for blocking mode
    //--------------------------------------------------------------------------

    if (GB_Global.mode == GrB_BLOCKING)
    {
        // delete any lingering zombies and assemble any pending tuples
        APPLY_PENDING_UPDATES (A) ;
    }
    return (REPORT_SUCCESS) ;
}

