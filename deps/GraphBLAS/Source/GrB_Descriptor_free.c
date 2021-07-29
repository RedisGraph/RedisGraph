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
        // only free a dynamically-allocated operator
        GrB_Descriptor desc = *descriptor ;
        if (desc != NULL)
        {
            size_t header_size = desc->header_size ;
            if (header_size > 0)
            { 
                GB_FREE (&(desc->logger), desc->logger_size) ;
                desc->logger_size = 0 ;
                desc->magic = GB_FREED ;  // to help detect dangling pointers
                desc->header_size = 0 ;
                GB_FREE (descriptor, header_size) ;
            }
        }
    }

    return (GrB_SUCCESS) ;
}

