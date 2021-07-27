//------------------------------------------------------------------------------
// GrB_Type_free:  free a user-defined type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Type_free          // free a user-defined type
(
    GrB_Type *type              // handle of user-defined type to free
)
{

    if (type != NULL)
    {
        // only free a dynamically-allocated type
        GrB_Type t = *type ;
        if (t != NULL)
        {
            size_t header_size = t->header_size ;
            if (header_size > 0)
            { 
                t->magic = GB_FREED ;  // to help detect dangling pointers
                t->header_size = 0 ;
                GB_FREE (type, header_size) ;
            }
        }
    }

    return (GrB_SUCCESS) ;
}

