//------------------------------------------------------------------------------
// GrB_Semiring_free: free a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

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
        if (s != NULL && !s->semiring_is_builtin)
        {
            if (s->magic == GB_MAGIC)
            { 
                // only user-defined semirings are freed.  predefined semirings
                // are statically allocated and cannot be freed.
                s->magic = GB_FREED ; // to help detect dangling pointers
                GB_FREE (*semiring) ;
            }
            (*semiring) = NULL ;
        }
    }

    return (GrB_SUCCESS) ;
}

