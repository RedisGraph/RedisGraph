//------------------------------------------------------------------------------
// GrB_Monoid_free:  free a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Monoid_free            // free a user-created monoid
(
    GrB_Monoid *monoid              // handle of monoid to free
)
{

    if (monoid != NULL)
    {
        // only free a dynamically-allocated monoid
        GrB_Monoid mon = *monoid ;
        if (mon != NULL)
        {
            size_t header_size = mon->header_size ;
            if (header_size > 0)
            { 
                mon->magic = GB_FREED ;  // to help detect dangling pointers
                mon->header_size = 0 ;
                GB_FREE (&(mon->identity), mon->identity_size) ;
                GB_FREE (&(mon->terminal), mon->terminal_size) ;
                GB_FREE (monoid, header_size) ;
            }
        }
    }

    return (GrB_SUCCESS) ;
}

