//------------------------------------------------------------------------------
// GB_Pending_ensure: ensure the list of pending tuples is large enough 
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_Pending.h"

bool GB_Pending_ensure      // create or reallocate a list of pending tuples
(
    GB_Pending *PHandle,    // input/output
    GrB_Type type,          // type of pending tuples
    GrB_BinaryOp op,        // operator for assembling pending tuples
    bool is_matrix,         // true if Pending->j must be allocated
    int64_t nnew            // # of pending tuples to add
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (PHandle != NULL) ;

    //--------------------------------------------------------------------------
    // ensure the list of pending tuples is large enough
    //--------------------------------------------------------------------------

    if ((*PHandle) == NULL)
    {
        if (!GB_Pending_alloc (PHandle, type, op, is_matrix, nnew))
        {
            // out of memory
            return (false) ;
        }
    }
    else if (!GB_Pending_realloc (PHandle, nnew))
    {
        // out of memory
        return (false) ;
    }

    return (true) ;
}

