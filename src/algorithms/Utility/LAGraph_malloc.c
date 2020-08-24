//------------------------------------------------------------------------------
// LAGraph_malloc:  wrapper for malloc and calloc
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// LAGraph_malloc:  wrapper for malloc, contributed by Tim Davis, Texas A&M

// Wrapper for malloc and calloc.

// TODO also need a wrapper for realloc.

#include "LAGraph_internal.h"

//------------------------------------------------------------------------------
// global space
//------------------------------------------------------------------------------

// These are modified by LAGraph_init and LAGraph_xinit.

void * (* LAGraph_malloc_function  ) (size_t)         = malloc ;
void * (* LAGraph_calloc_function  ) (size_t, size_t) = calloc ;
void * (* LAGraph_realloc_function ) (void *, size_t) = realloc ;
void   (* LAGraph_free_function    ) (void *)         = free ;

bool LAGraph_malloc_is_thread_safe =
    #ifdef MATLAB_MEX_FILE
        false       // mxMalloc is not thread-safe
    #else
        true        // ANSI C malloc, TBB scalable_malloc, etc are thread safe
    #endif
        ;

//------------------------------------------------------------------------------
// LAGraph_malloc
//------------------------------------------------------------------------------

void *LAGraph_malloc
(
    size_t nitems,          // number of items
    size_t size_of_item     // size of each item
)
{

    // make sure at least one item is allocated
    nitems = LAGRAPH_MAX (1, nitems) ;

    // make sure at least one byte is allocated
    size_of_item = LAGRAPH_MAX (1, size_of_item) ;

    // check for integer overflow
    if ((double) nitems * (double) size_of_item > (double) INT64_MAX)
    {
        return (NULL) ;
    }

    // malloc the space
    return (LAGraph_malloc_function (nitems * size_of_item)) ;
}

//------------------------------------------------------------------------------
// LAGraph_calloc
//------------------------------------------------------------------------------

void *LAGraph_calloc
(
    size_t nitems,          // number of items
    size_t size_of_item     // size of each item
)
{

    // make sure at least one item is allocated
    nitems = LAGRAPH_MAX (1, nitems) ;

    // make sure at least one byte is allocated
    size_of_item = LAGRAPH_MAX (1, size_of_item) ;

    // check for integer overflow
    if ((double) nitems * (double) size_of_item > (double) INT64_MAX)
    {
        return (NULL) ;
    }

    // calloc the space
    return (LAGraph_calloc_function (nitems, size_of_item)) ;
}

