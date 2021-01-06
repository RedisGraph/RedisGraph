//------------------------------------------------------------------------------
// GB_realloc_memory: wrapper for realloc_function
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A wrapper for realloc_function.

// If p is non-NULL on input, it points to a previously allocated object of
// size nitems_old * size_of_item.  The object is reallocated to be of size
// nitems_new * size_of_item.  If p is NULL on input, then a new object of that
// size is allocated.  On success, a pointer to the new object is returned, and
// ok is returned as true.  If the allocation fails, ok is set to false and a
// pointer to the old (unmodified) object is returned.

// Usage:

//      p = GB_realloc_memory (nnew, nold, size, p, &ok)

//      if (ok)

//          p points to a space of size at least nnew*size, and the first
//          part, of size min(nnew,nold)*size, has the same content as
//          the old memory space if it was present.

//      else

//          p points to the old space of size nold*size, which is left
//          unchanged.  This case never occurs if nnew < nold.

#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
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

    size_t size, oldsize ;

    // make sure at least one item is allocated
    nitems_old = GB_IMAX (1, nitems_old) ;
    nitems_new = GB_IMAX (1, nitems_new) ;

    // make sure at least one byte is allocated
    size_of_item = GB_IMAX (1, size_of_item) ;

    (*ok) = GB_size_t_multiply (&size,    nitems_new, size_of_item)
         && GB_size_t_multiply (&oldsize, nitems_old, size_of_item) ;

    if (!(*ok) || nitems_new > GxB_INDEX_MAX || size_of_item > GxB_INDEX_MAX)
    { 
        // overflow
        (*ok) = false ;
    }
    else if (p == NULL)
    { 
        // a fresh object is being allocated
        p = (void *) GB_malloc_memory (nitems_new, size_of_item) ;
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
        
        //----------------------------------------------------------------------
        // for memory usage testing only
        //----------------------------------------------------------------------

        bool pretend_to_fail = false ;
        if (GB_Global_malloc_tracking_get ( ))
        { 
            // brutal memory debug; pretend to fail if (count-- <= 0).
            if (GB_Global_malloc_debug_get ( ))
            {
                pretend_to_fail = GB_Global_malloc_debug_count_decrement ( ) ;
            }
        }

        //----------------------------------------------------------------------
        // reallocate the memory
        //----------------------------------------------------------------------

        if (pretend_to_fail)
        {
            pnew = NULL ;
        }
        else
        { 

            //------------------------------------------------------------------
            // reallocate the space
            //------------------------------------------------------------------

            if (GB_Global_have_realloc_function ( ))
            {

                //--------------------------------------------------------------
                // use realloc
                //--------------------------------------------------------------

                pnew = (void *) GB_Global_realloc_function (p, size) ;

            }
            else
            {

                //--------------------------------------------------------------
                // no realloc function: mimic with malloc and memcpy
                //--------------------------------------------------------------

                // malloc the new space
                pnew = (void *) GB_Global_malloc_function (size) ;

                // copy over the data from the old space to the new space
                if (pnew != NULL)
                {
                    // TODO for GPU: use a parallel memcpy
                    memcpy (pnew, p, GB_IMIN (oldsize, size)) ;
                }

                // free the old space
                GB_Global_free_function (p) ;
            }
        }


        //----------------------------------------------------------------------
        // check if successful
        //----------------------------------------------------------------------

        if (pnew == NULL)
        {
            if (nitems_new < nitems_old)
            {
                // the attempt to reduce the size of the block failed, but
                // the old block is unchanged.  So pretend to succeed.
                (*ok) = true ;
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
        }

    }
    return (p) ;
}

