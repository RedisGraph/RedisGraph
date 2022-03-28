//------------------------------------------------------------------------------
// GB_dealloc_memory: wrapper for free, using the free_pool
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A wrapper for free.  If p is NULL on input, it is not freed.

// The memory is freed by returning it to the free_pool if it is small enough
// and an exact power of two.  Otherwise, it is freed via GB_free_memory,
// and not returned to the free_pool.

#include "GB.h"

GB_PUBLIC
void GB_dealloc_memory      // free memory, return to free_pool or free it
(
    // input/output
    void **p,               // pointer to allocated block of memory to free
    // input
    size_t size_allocated   // # of bytes actually allocated
)
{

    if (p != NULL && (*p) != NULL)
    {
        bool returned_to_free_pool = false ;

        if (GB_IS_POWER_OF_TWO (size_allocated))
        { 

            //------------------------------------------------------------------
            // return the memory to the free_pool, if possible
            //------------------------------------------------------------------

            int k = GB_CEIL_LOG2 (size_allocated) ;
            if (GB_Global_free_pool_limit_get (k) > 0)
            {
                #ifdef GB_MEMDUMP
                printf ("put to free pool %p %d\n", *p, k) ;
                #endif
                returned_to_free_pool = GB_Global_free_pool_put (*p, k) ;
            }
        }

        if (!returned_to_free_pool)
        { 

            //------------------------------------------------------------------
            // otherwise free the memory back to the memory manager
            //------------------------------------------------------------------

            GB_free_memory (p, size_allocated) ;
        }

        #ifdef GB_MEMDUMP
        GB_Global_free_pool_dump (2) ; GB_Global_memtable_dump ( ) ;
        #endif

        (*p) = NULL ;
    }
}

