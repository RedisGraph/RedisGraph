//------------------------------------------------------------------------------
// GB_block: apply all pending computations if blocking mode enabled
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_Pending.h"

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
    // check for blocking mode
    //--------------------------------------------------------------------------

    if (GB_shall_block (A))
    { 
        // delete any lingering zombies and assemble any pending tuples
        GB_WAIT (A) ;
    }
    return (GrB_SUCCESS) ;
}

