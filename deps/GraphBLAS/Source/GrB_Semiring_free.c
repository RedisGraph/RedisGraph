//------------------------------------------------------------------------------
// GrB_Semiring_free: free a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Semiring_free          // free a user-created semiring
(
    GrB_Semiring *semiring          // handle of semiring to free
)
{

    if (semiring != NULL)
    {
        GrB_Semiring s = *semiring ;
        if (s != NULL && s->object_kind == GB_USER_RUNTIME)
        {
            if (s->magic == GB_MAGIC)
            { 
                // only user-defined semirings are freed.  predefined semirings
                // are statically allocated and cannot be freed.
                s->magic = GB_FREED ; // to help detect dangling pointers
                GB_FREE_MEMORY (*semiring, 1,
                    sizeof (struct GB_Semiring_opaque)) ;
            }
            (*semiring) = NULL ;
        }
    }

    return (GrB_SUCCESS) ;
}

