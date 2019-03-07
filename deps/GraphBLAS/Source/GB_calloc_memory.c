//------------------------------------------------------------------------------
// GB_calloc_memory: wrapper for calloc (used via the GB_CALLOC_MEMORY macro)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A wrapper for calloc.  Space is set to zero.

// This function is called via the GB_CALLOC_MEMORY(p,n,s,Context) macro.

// Parameters are the same as the POSIX calloc, except that asking to allocate
// a block of zero size causes a block of size 1 to be allocated instead.  This
// allows the return pointer p to be checked for the out-of-memory condition,
// even when allocating an object of size zero.

// PARALLEL: clear the array in parallel?

#include "GB.h"

void *GB_calloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item,    // sizeof each item
    GB_Context Context      // for # of threads.  Use one thread if NULL
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

        // determine the number of threads to use
        GB_GET_NTHREADS (nthreads, Context) ;

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
                p = (void *) GB_Global.calloc_function (nitems, size_of_item) ;
            }
            if (p != NULL)
            {
                int nmalloc = GB_Global_nmalloc_increment ( ) ;
                GB_Global_inuse_increment (nitems * size_of_item) ;
                #ifdef GB_PRINT_MALLOC
                printf ("Calloc:  %14p %3d %1d n "GBd" size "GBd"\n",
                    p, nmalloc, GB_Global.malloc_debug,
                    (int64_t) nitems, (int64_t) size_of_item) ;
                #endif
            }
        }
        else
        {
            // normal use, in production
            p = (void *) GB_Global.calloc_function (nitems, size_of_item) ;
        }

    }
    return (p) ;
}

