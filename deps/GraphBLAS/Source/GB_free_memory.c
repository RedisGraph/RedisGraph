//------------------------------------------------------------------------------
// GB_free_memory: wrapper for free (used via the GB_FREE_MEMORY macro)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A wrapper for free.  If p is NULL on input, it is not freed.

// This function is called via the GB_FREE_MEMORY(p,n,s) macro.

// to turn on memory usage debug printing, uncomment this line:
// #define GB_PRINT_MALLOC 1

#include "GB.h"

void GB_free_memory
(
    void *p,                // pointer to allocated block of memory to free
    size_t nitems,          // number of items to free
    size_t size_of_item     // sizeof each item
)
{
    if (p != NULL)
    { 

        if (GB_Global_malloc_tracking_get ( ))
        {

            //------------------------------------------------------------------
            // for memory usage testing only
            //------------------------------------------------------------------

            // at least one item is always allocated
            nitems = GB_IMAX (1, nitems) ;
            size_of_item = GB_IMAX (1, size_of_item) ;

            #if defined ( GB_PRINT_MALLOC ) || defined ( GB_DEBUG )

                int nmalloc = 0 ;
                #define GB_CRITICAL_SECTION                             \
                {                                                       \
                    nmalloc = GB_Global_nmalloc_decrement ( ) ;         \
                    GB_Global_inuse_decrement (nitems * size_of_item) ; \
                }

            #else

                #define GB_CRITICAL_SECTION                             \
                {                                                       \
                    GB_Global_nmalloc_decrement ( ) ;                   \
                    GB_Global_inuse_decrement (nitems * size_of_item) ; \
                }

            #endif

            #if defined (USER_POSIX_THREADS) || defined (USER_ANSI_THREADS)
            bool ok = true ;
            #endif
            #include "GB_critical_section.c"

            #ifdef GB_PRINT_MALLOC
            printf ("%14p Free:  %3d %1d n "GBd" size "GBd"\n",
                p, nmalloc, GB_Global_malloc_debug_get ( ), (int64_t) nitems,
                (int64_t) size_of_item) ;
            if (nmalloc < 0)
            {
                printf ("%d free    %p negative mallocs!\n", nmalloc, p) ;
            }
            #endif

            ASSERT (nmalloc >= 0) ;
        }

        //----------------------------------------------------------------------
        // free the memory
        //----------------------------------------------------------------------

        GB_Global_free_function (p) ;
    }
}

