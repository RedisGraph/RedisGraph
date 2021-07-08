//------------------------------------------------------------------------------
// GrB_BinaryOp_free: free a binary operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_BinaryOp_free          // free a user-created binary operator
(
    GrB_BinaryOp *binaryop          // handle of binary operator to free
)
{

    if (binaryop != NULL)
    {
        // only free a dynamically-allocated operator
        GrB_BinaryOp op = *binaryop ;
        if (op != NULL)
        {
            size_t header_size = op->header_size ;
            if (header_size > 0)
            { 
                op->magic = GB_FREED ;  // to help detect dangling pointers
                op->header_size = 0 ;
                GB_FREE (binaryop, header_size) ;
            }
        }
    }

    return (GrB_SUCCESS) ;
}

