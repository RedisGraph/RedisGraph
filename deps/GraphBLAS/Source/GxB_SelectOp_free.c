//------------------------------------------------------------------------------
// GxB_SelectOp_free: free a select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_SelectOp_free          // free a user-created select operator
(
    GxB_SelectOp *selectop          // handle of select operator to free
)
{

    if (selectop != NULL)
    {
        // only free a dynamically-allocated operator
        GxB_SelectOp op = *selectop ;
        if (op != NULL)
        {
            size_t header_size = op->header_size ;
            if (header_size > 0)
            { 
                op->magic = GB_FREED ;  // to help detect dangling pointers
                op->header_size = 0 ;
                GB_FREE (selectop, header_size) ;
            }
        }
    }

    return (GrB_SUCCESS) ;
}

