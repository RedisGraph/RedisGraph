//------------------------------------------------------------------------------
// GrB_Type_free:  free a user-defined type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
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
        // only free a dynamically-allocated type, which have header_size > 0
        GrB_Type t = *type ;
        if (t != NULL)
        {
            size_t header_size = t->header_size ;
            if (header_size > 0)
            { 
                size_t defn_size = t->defn_size ;
                if (defn_size > 0)
                { 
                    GB_FREE (&(t->defn), defn_size) ;
                }
                t->magic = GB_FREED ;  // to help detect dangling pointers
                t->header_size = 0 ;
                GB_FREE (type, header_size) ;
            }
        }
    }

    return (GrB_SUCCESS) ;
}

