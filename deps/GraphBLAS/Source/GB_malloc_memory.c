//------------------------------------------------------------------------------
// GB_malloc_memory: wrapper for malloc_function
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A wrapper for malloc_function.  Space is not initialized.

// This function is called via the GB_MALLOC_MEMORY(p,n,s) macro.

// Asking to allocate a block of zero size causes a block of size 1 to be
// allocated instead.  This allows the return pointer p to be checked for the
// out-of-memory condition, even when allocating an object of size zero.

// not parallel: this function does O(1) work and is already thread-safe.

#include "GB.h"

void *GB_malloc_memory      // pointer to allocated block of memory
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
    if (!ok || nitems > GB_INDEX_MAX || size_of_item > GB_INDEX_MAX)
    { 
        // overflow
        p = NULL ;
    }
    else
    { 

        if (GB_Global_malloc_tracking_get ( ))
        {
            // for memory usage testing only
            bool pretend_to_fail = false ;
            if (GB_Global.malloc_debug)
            {
                // brutal memory usage debug; pretend to fail if the count <= 0
                pretend_to_fail = (GB_Global.malloc_debug_count-- <= 0) ;
            }
            if (pretend_to_fail)
            {
                #ifdef GB_PRINT_MALLOC
                printf ("pretend to fail\n") ;
                #endif
                p = NULL ;
            }
            else
            {
                p = (void *) GB_Global.malloc_function (size) ;
            }
            if (p != NULL)
            {
                int nmalloc = GB_Global_nmalloc_increment ( ) ;
                GB_Global_inuse_increment (nitems * size_of_item) ;
                #ifdef GB_PRINT_MALLOC
                printf ("Malloc:  %14p %3d %1d n "GBd" size "GBd"\n",
                    p, nmalloc, GB_Global.malloc_debug,
                    (int64_t) nitems, (int64_t) size_of_item) ;
                #endif
            }
        }
        else
        {
            // normal use, in production
            p = (void *) GB_Global.malloc_function (size) ;
        }

    }
    return (p) ;
}

