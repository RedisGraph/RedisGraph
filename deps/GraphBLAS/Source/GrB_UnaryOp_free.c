//------------------------------------------------------------------------------
// GrB_UnaryOp_free: free a unary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_UnaryOp_free           // free a user-created unary operator
(
    GrB_UnaryOp *unaryop            // handle of unary operator to free
)
{

    if (unaryop != NULL)
    {
        // only free a dynamically-allocated operator
        GrB_UnaryOp op = *unaryop ;
        if (op != NULL)
        {
            size_t header_size = op->header_size ;
            if (header_size > 0)
            { 
                op->magic = GB_FREED ;  // to help detect dangling pointers
                op->header_size = 0 ;
                GB_FREE (unaryop, header_size) ;
            }
        }
    }

    return (GrB_SUCCESS) ;
}

