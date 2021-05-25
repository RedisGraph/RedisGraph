//------------------------------------------------------------------------------
// GB_calloc_memory: wrapper for calloc_function
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A wrapper for calloc_function.  Space is set to zero.

// Parameters are the same as the ANSI C11 calloc, except that asking to
// allocate a block of zero size causes a block of size 1 to be allocated
// instead.  This allows the return pointer p to be checked for the
// out-of-memory condition, even when allocating an object of size zero.

#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void *GB_calloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item     // sizeof each item
)
{

    void *p ;
    size_t size ;

    // make sure at least one item is allocated
    nitems = GB_IMAX (1, nitems) ;

    // make sure at least one byte is allocated
    size_of_item = GB_IMAX (1, size_of_item) ;

    bool ok = GB_size_t_multiply (&size, nitems, size_of_item) ;
    if (!ok || nitems > GxB_INDEX_MAX || size_of_item > GxB_INDEX_MAX)
    { 
        // overflow
        p = NULL ;
    }
    else
    { 

        if (GB_Global_malloc_tracking_get ( ))
        {

            //------------------------------------------------------------------
            // for memory usage testing only
            //------------------------------------------------------------------

            // brutal memory debug; pretend to fail if (count-- <= 0).
            bool pretend_to_fail = false ;
            if (GB_Global_malloc_debug_get ( ))
            {
                pretend_to_fail = GB_Global_malloc_debug_count_decrement ( ) ;
            }

            // allocate the memory
            if (pretend_to_fail)
            { 
                p = NULL ;
            }
            else
            { 
                p = (void *) GB_Global_calloc_function (nitems, size_of_item) ;
            }

            // check if successful
            if (p != NULL)
            { 
                // success
                GB_Global_nmalloc_increment ( ) ;
            }

        }
        else
        { 

            //------------------------------------------------------------------
            // normal use, in production
            //------------------------------------------------------------------

            p = (void *) GB_Global_calloc_function (nitems, size_of_item) ;
        }

    }
    return (p) ;
}

