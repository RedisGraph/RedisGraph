//------------------------------------------------------------------------------
// LAGraph_Free:  wrapper for free
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
// Contributed by Tim Davis, Texas A&M University.

//------------------------------------------------------------------------------

// LAGraph_Free frees a block of memory obtained by LAGraph_Malloc.  It does
// nothing if p is NULL.

#include "LG_internal.h"

void LAGraph_Free
(
    void **p                // pointer to object to free, does nothing if NULL
)
{

    if (p != NULL && (*p) != NULL)
    {
        LAGraph_Free_function (*p) ;
        (*p) = NULL ;
    }
}
