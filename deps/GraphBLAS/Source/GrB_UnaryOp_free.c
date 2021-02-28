//------------------------------------------------------------------------------
// GrB_UnaryOp_free: free a unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_UnaryOp_free           // free a user-created unary operator
(
    GrB_UnaryOp *unaryop            // handle of unary operator to free
)
{

    if (unaryop != NULL)
    {
        // only free a user-defined operator
        GrB_UnaryOp op = *unaryop ;
        if (op != NULL && op->opcode == GB_USER_opcode)
        {
            if (op->magic == GB_MAGIC)
            { 
                op->magic = GB_FREED ;  // to help detect dangling pointers
                GB_FREE_MEMORY (*unaryop, 1, sizeof (struct GB_UnaryOp_opaque));
            }
            (*unaryop) = NULL ;
        }
    }

    return (GrB_SUCCESS) ;
}

