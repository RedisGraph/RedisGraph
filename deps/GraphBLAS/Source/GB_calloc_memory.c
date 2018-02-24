//------------------------------------------------------------------------------
// GB_calloc_memory: wrapper for calloc
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A wrapper for CALLOC.  Space is set to zero.

// Parameters are the same as the POSIX calloc, except that asking to allocate
// a block of zero size causes a block of size 1 to be allocated instead.  This
// allows the return pointer p to be checked for the out-of-memory condition,
// even when allocating an object of size zero.

// By default, CALLOC is defined in GB.h as calloc.  For a MATLAB mexFunction,
// it is mxCalloc.  It can also be defined at compile time with
// -DCALLOC=mycallocfunc.

#include "GB.h"

void *GB_calloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item     // sizeof each item
)
{

    void *p ;
    size_t size ;
    int nmalloc ;

    // make sure at least one item is allocated
    nitems = IMAX (1, nitems) ;

    // make sure at least one byte is allocated
    size_of_item = IMAX (1, size_of_item) ;

    bool ok = GB_size_t_multiply (&size, nitems, size_of_item) ;
    if (!ok || nitems > GB_INDEX_MAX || size_of_item > GB_INDEX_MAX)
    {
        // overflow
        p = NULL ;
    }
    else
    {

        // check the malloc debug status.  This debug flag is set outside
        // of GraphBLAS and not modified, so it is safe to check it outside
        // a critical section.
        bool pretend_to_fail = false ;
        if (GB_Global.malloc_debug)
        {
            // brutal malloc debug; pretend to fail if the count <= 0
            #pragma omp critical (GB_memory)
            {
                pretend_to_fail = (GB_Global.malloc_debug_count-- <= 0) ;
            }
        }

        if (pretend_to_fail)
        {
            p = NULL ;
        }
        else
        {
            p = (void *) CALLOC (nitems, size_of_item) ;
        }

        if (p != NULL)
        {
            #pragma omp critical (GB_memory)
            {
                nmalloc = ++GB_Global.nmalloc ;
                GB_Global.inuse += nitems * size_of_item ;
                GB_Global.maxused = IMAX (GB_Global.maxused, GB_Global.inuse) ;
            }

#ifdef PRINT_MALLOC
            printf ("calloc:  %14p %3d %1d n "GBd" size "GBd"\n", 
                p, nmalloc, GB_Global.malloc_debug,
                (int64_t) nitems, (int64_t) size_of_item) ;
#endif
        }
    }
    return (p) ;
}

