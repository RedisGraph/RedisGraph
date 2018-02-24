//------------------------------------------------------------------------------
// GxB_SelectOp_free: free a select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_SelectOp_free          // free a user-created select operator
(
    GxB_SelectOp *selectop          // handle of select operator to free
)
{

    if (selectop != NULL)
    {
        // only free a user-defined operator, not a built-in one
        GxB_SelectOp op = *selectop ;
        if (op != NULL && op->opcode == GB_USER_SELECT_opcode)
        {
            if (op->magic == MAGIC)
            {
                op->magic = FREED ;         // to help detect dangling pointers
                GB_FREE_MEMORY (*selectop, 1, sizeof (GB_SelectOp_opaque)) ;
            }
            (*selectop) = NULL ;
        }
    }

    return (GrB_SUCCESS) ;
}

