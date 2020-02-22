//------------------------------------------------------------------------------
// GrB_Monoid_free:  free a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
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
        if (mon != NULL && mon->object_kind == GB_USER_RUNTIME)
        {
            if (mon->magic == GB_MAGIC)
            { 
                // only user-defined monoids are freed.  predefined monoids
                // are statically allocated and cannot be freed.
                mon->magic = GB_FREED ; // to help detect dangling pointers
                // mon->op->ztype->size might not be safe if op or ztype are
                // user-defined and have already been freed; use op_ztype_size.
                size_t zsize = mon->op_ztype_size ;
                GB_FREE_MEMORY (mon->identity, 1, zsize) ;
                GB_FREE_MEMORY (mon->terminal, 1, zsize) ;
                GB_FREE_MEMORY (*monoid, 1, sizeof (struct GB_Monoid_opaque)) ;
            }
            (*monoid) = NULL ;
        }
    }

    return (GrB_SUCCESS) ;
}

