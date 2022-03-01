//------------------------------------------------------------------------------
// GB_werk.h: definitions for werkspace management on the Werk stack
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_WERK_H
#define GB_WERK_H

//------------------------------------------------------------------------------
// GB_werk_push/pop: manage werkspace in the Context->Werk stack
//------------------------------------------------------------------------------

// Context->Werk is a small fixed-size array that is allocated on the stack
// of any user-callable GraphBLAS function.  It is used for small werkspace
// allocations.

// GB_ROUND8(s) rounds up s to a multiple of 8
#define GB_ROUND8(s) (((s) + 7) & (~0x7))

//------------------------------------------------------------------------------
// GB_werk_push: allocate werkspace from the Werk stack or malloc
//------------------------------------------------------------------------------

// The werkspace is allocated from the Werk static if it small enough and space
// is available.  Otherwise it is allocated by malloc.

static inline void *GB_werk_push    // return pointer to newly allocated space
(
    // output
    size_t *size_allocated,         // # of bytes actually allocated
    bool *on_stack,                 // true if werkspace is from Werk stack
    // input
    size_t nitems,                  // # of items to allocate
    size_t size_of_item,            // size of each item
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (on_stack != NULL) ;
    ASSERT (size_allocated != NULL) ;

    //--------------------------------------------------------------------------
    // determine where to allocate the werkspace
    //--------------------------------------------------------------------------

    size_t size ;
    if (Context == NULL || nitems > GB_WERK_SIZE || size_of_item > GB_WERK_SIZE
        #ifdef GBCOVER
        // Werk stack can be disabled for test coverage
        || (GB_Global_hack_get (1) != 0)
        #endif
    )
    { 
        // no context, or werkspace is too large to allocate from the Werk stack
        (*on_stack) = false ;
    }
    else
    { 
        // try to allocate from the Werk stack
        size = GB_ROUND8 (nitems * size_of_item) ;
        ASSERT (size % 8 == 0) ;        // size is rounded up to a multiple of 8
        size_t freespace = GB_WERK_SIZE - Context->pwerk ;
        ASSERT (freespace % 8 == 0) ;   // thus freespace is also multiple of 8
        (*on_stack) = (size <= freespace) ;
    }

    //--------------------------------------------------------------------------
    // allocate the werkspace
    //--------------------------------------------------------------------------

    if (*on_stack)
    { 
        // allocate the werkspace from the Werk stack
        GB_void *p = Context->Werk + Context->pwerk ;
        Context->pwerk += (int) size ;
        (*size_allocated) = size ;
        return ((void *) p) ;
    }
    else
    { 
        // allocate the werkspace from malloc
        return (GB_malloc_memory (nitems, size_of_item, size_allocated)) ;
    }
}

//------------------------------------------------------------------------------
// GB_WERK helper macros
//------------------------------------------------------------------------------

// declare a werkspace X of a given type
#define GB_WERK_DECLARE(X,type)                     \
    type *restrict X = NULL ;                    \
    bool X ## _on_stack = false ;                   \
    size_t X ## _nitems = 0, X ## _size_allocated = 0 ;

// push werkspace X
#define GB_WERK_PUSH(X,nitems,type)                                         \
    X ## _nitems = (nitems) ;                                               \
    X = (type *) GB_werk_push (&(X ## _size_allocated), &(X ## _on_stack),  \
        X ## _nitems, sizeof (type), Context) ; 

// pop werkspace X
#define GB_WERK_POP(X,type)                                                 \
    X = (type *) GB_werk_pop (X, &(X ## _size_allocated), X ## _on_stack,   \
        X ## _nitems, sizeof (type), Context) ; 

//------------------------------------------------------------------------------
// GB_werk_pop:  free werkspace from the Werk stack
//------------------------------------------------------------------------------

// If the werkspace was allocated from the Werk stack, it must be at the top of
// the stack to free it properly.  Freeing a werkspace in the middle of the
// Werk stack also frees everything above it.  This is not a problem if that
// space is also being freed, but the assertion below ensures that the freeing
// werkspace from the Werk stack is done in LIFO order, like a stack.

static inline void *GB_werk_pop     // free the top block of werkspace memory
(
    // input/output
    void *p,                        // werkspace to free
    size_t *size_allocated,         // # of bytes actually allocated for p
    // input
    bool on_stack,                  // true if werkspace is from Werk stack
    size_t nitems,                  // # of items to allocate
    size_t size_of_item,            // size of each item
    GB_Context Context
)
{
    ASSERT (size_allocated != NULL) ;

    if (p == NULL)
    { 
        // nothing to do
    }
    else if (on_stack)
    { 
        // werkspace was allocated from the Werk stack
        ASSERT ((*size_allocated) == GB_ROUND8 (nitems * size_of_item)) ;
        ASSERT (Context != NULL) ;
        ASSERT ((*size_allocated) % 8 == 0) ;
        ASSERT (((GB_void *) p) + (*size_allocated) ==
                Context->Werk + Context->pwerk) ;
        Context->pwerk = ((GB_void *) p) - Context->Werk ;
        (*size_allocated) = 0 ;
    }
    else
    { 
        // werkspace was allocated from malloc
        GB_dealloc_memory (&p, *size_allocated) ;
    }
    return (NULL) ;                 // return NULL to indicate p was freed
}

#endif

