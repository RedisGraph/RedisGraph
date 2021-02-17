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
        GrB_Monoid mon = *monoid ;
        if (mon != NULL && !mon->monoid_is_builtin)
        {
            if (mon->magic == GB_MAGIC)
            { 
                // only user-defined monoids are freed.  predefined monoids
                // are statically allocated and cannot be freed.
                mon->magic = GB_FREED ; // to help detect dangling pointers
                GB_FREE (mon->identity) ;   // ok if already NULL
                GB_FREE (mon->terminal) ;   // ok if already NULL
                GB_FREE (*monoid) ;
            }
            (*monoid) = NULL ;
        }
    }

    return (GrB_SUCCESS) ;
}

