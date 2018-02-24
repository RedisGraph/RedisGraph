//------------------------------------------------------------------------------
// GrB_Semiring_free: free a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
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
        if (s != NULL && s->user_defined)
        {
            if (s->magic == MAGIC)
            {
                // only user-defined semirings are freed.  predefined semirings
                // are statically allocated and cannot be freed.
                s->magic = FREED ; // to help detect dangling pointers
                GB_FREE_MEMORY (*semiring, 1, sizeof (GB_Semiring_opaque)) ;
            }
            (*semiring) = NULL ;
        }
    }

    return (GrB_SUCCESS) ;
}

