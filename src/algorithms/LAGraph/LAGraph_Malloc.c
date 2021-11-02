//------------------------------------------------------------------------------
// LAGraph_Malloc:  wrapper for malloc
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
// Contributed by Tim Davis, Texas A&M University.

//------------------------------------------------------------------------------

#include "LG_internal.h"

void *LAGraph_Malloc
(
    size_t nitems,          // number of items
    size_t size_of_item     // size of each item
)
{

    // make sure at least one item is allocated
    nitems = LAGraph_MAX (1, nitems) ;

    // make sure at least one byte is allocated
    size_of_item = LAGraph_MAX (1, size_of_item) ;

    // compute the size and check for integer overflow
    size_t size ;
    bool ok = LG_Multiply_size_t (&size, nitems, size_of_item) ;
    if (!ok || nitems > LAGRAPH_INDEX_MAX || size_of_item > LAGRAPH_INDEX_MAX)
    {
        // overflow
        return (NULL) ;
    }

    // malloc the space
    void *p = LAGraph_Malloc_function (size) ;
    return (p) ;
}
