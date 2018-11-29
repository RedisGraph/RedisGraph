//------------------------------------------------------------------------------
// GB_realloc_memory: wrapper for realloc
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A wrapper for realloc

// If p is non-NULL on input, it points to a previously allocated object of
// size nitems_old * size_of_item.  The object is reallocated to be of size
// nitems_new * size_of_item.  If p is NULL on input, then a new object of that
// size is allocated.  On success, a pointer to the new object is returned, and
// ok is returned as true.  If the allocation fails, ok is set to false and a
// pointer to the old (unmodified) object is returned.

// Usage:
//
//      p = GB_realloc_memory (nnew, nold, size, p, &ok)
//      if (ok)
//          p points to a space of size at least nnew*size, and the first
//          part, of size min(nnew,nold)*size, has the same content as
//          the old memory space if it was present.
//      else
//          p points to the old space of size nold*size, which is left
//          unchanged.  This case never occurs if nnew < nold.

// By default, GB_REALLOC is defined in GB.h as realloc.  For a MATLAB
// mexFunction, it is mxRealloc.  It can also be defined at compile time with
// -DGB_REALLOC=myreallocfunc.

#include "GB.h"

void *GB_realloc_memory     // pointer to reallocated block of memory, or
                            // to original block if the reallocation failed.
(
    size_t nitems_new,      // new number of items in the object
    size_t nitems_old,      // old number of items in the object
    size_t size_of_item,    // sizeof each item
    void *p,                // old object to reallocate
    bool *ok                // true if successful, false otherwise
)
{

    size_t size ;

    // make sure at least one item is allocated
    nitems_old = GB_IMAX (1, nitems_old) ;
    nitems_new = GB_IMAX (1, nitems_new) ;

    // make sure at least one byte is allocated
    size_of_item = GB_IMAX (1, size_of_item) ;

    (*ok) = GB_size_t_multiply (&size, nitems_new, size_of_item) ;
    if (!(*ok) || nitems_new > GB_INDEX_MAX || size_of_item > GB_INDEX_MAX)
    { 
        // overflow
        (*ok) = false ;
    }
    else if (p == NULL)
    { 
        // a fresh object is being allocated
        GB_MALLOC_MEMORY (p, nitems_new, size_of_item) ;
        (*ok) = (p != NULL) ;
    }
    else if (nitems_old == nitems_new)
    { 
        // the object does not change; do nothing
        (*ok) = true ;
    }
    else
    { 
        // change the size of the object from nitems_old to nitems_new
        void *pnew ;

        #ifdef GB_MALLOC_TRACKING
        bool pretend_to_fail = false ;
        if (GB_Global.malloc_debug)
        {
            // brutal malloc debug; pretend to fail if the count <= 0
            pretend_to_fail = (GB_Global.malloc_debug_count-- <= 0) ;
        }
        if (pretend_to_fail)
        {
            // brutal malloc debug; pretend to fail if the count <= 0,
            #ifdef GB_PRINT_MALLOC
            printf ("pretend to fail: realloc\n") ;
            #endif
            pnew = NULL ;
        }
        else
        #endif
        {
            // realloc the space
            pnew = (void *) GB_REALLOC (p, size) ;
        }

        if (pnew == NULL)
        {
            if (nitems_new < nitems_old)
            {
                // the attempt to reduce the size of the block failed, but
                // the old block is unchanged.  So pretend to succeed.
                (*ok) = true ;
                #ifdef GB_MALLOC_TRACKING
                GB_Global.inuse -= (nitems_old - nitems_new) * size_of_item;
                #endif
            }
            else
            {
                // out of memory
                (*ok) = false ;
            }
        }
        else
        {
            // success
            p = pnew ;
            (*ok) = true ;
            #ifdef GB_MALLOC_TRACKING
            GB_Global.inuse += (nitems_new - nitems_old) * size_of_item ;
            GB_Global.maxused = GB_IMAX (GB_Global.maxused, GB_Global.inuse) ;
            #endif
        }

        #ifdef GB_MALLOC_TRACKING
        #ifdef GB_PRINT_MALLOC
        printf ("realloc: %14p %3d %1d n "GBd" -> "GBd" size "GBd"\n",
            pnew, GB_Global.nmalloc, GB_Global.malloc_debug,
            (int64_t) nitems_old, (int64_t) nitems_new,
            (int64_t) size_of_item) ;
        #endif
        #endif

    }
    return (p) ;
}

