//------------------------------------------------------------------------------
// GrB_Monoid_free:  free a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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
        if (mon != NULL && mon->user_defined)
        {
            if (mon->magic == MAGIC)
            {
                // only user-defined monoids are freed.  predefined monoids
                // are statically allocated and cannot be freed.
                mon->magic = FREED ; // to help detect dangling pointers
                GB_FREE_MEMORY (mon->identity, 1, mon->op->ztype->size) ;
                GB_FREE_MEMORY (*monoid, 1, sizeof (GB_Monoid_opaque)) ;
            }
            (*monoid) = NULL ;
        }
    }

    return (GrB_SUCCESS) ;
}

