//------------------------------------------------------------------------------
// GrB_Descriptor_free: free a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Predefined descriptors are not freed.  Attempts to do so are silently
// ignored.

#include "GB.h"

GrB_Info GrB_Descriptor_free            // free a descriptor
(
    GrB_Descriptor *descriptor          // handle of descriptor to free
)
{

    if (descriptor != NULL)
    {
        GrB_Descriptor desc = *descriptor ;
        if (desc != NULL && desc->magic == GB_MAGIC && !(desc->predefined))
        { 
            GB_FREE (desc->logger) ;    // free the error logger string
            desc->magic = GB_FREED ;    // to help detect dangling pointers
            GB_FREE (*descriptor) ;
        }
        (*descriptor) = NULL ;
    }

    return (GrB_SUCCESS) ;
}

