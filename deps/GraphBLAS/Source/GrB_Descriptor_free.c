//------------------------------------------------------------------------------
// GrB_Descriptor_free: free a descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Descriptor_free            // free a descriptor
(
    GrB_Descriptor *descriptor          // handle of descriptor to free
)
{

    if (descriptor != NULL)
    {
        GrB_Descriptor desc = *descriptor ;
        if (desc != NULL && desc->magic == GB_MAGIC)
        { 
            desc->magic = GB_FREED ;     // to help detect dangling pointers
            GB_FREE_MEMORY (*descriptor, 1,
                sizeof (struct GB_Descriptor_opaque)) ;
        }
        (*descriptor) = NULL ;
    }

    return (GrB_SUCCESS) ;
}

