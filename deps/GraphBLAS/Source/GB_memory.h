//------------------------------------------------------------------------------
// GB_memory.h: memory allocation
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_MEMORY_H
#define GB_MEMORY_H

//------------------------------------------------------------------------------
// memory management
//------------------------------------------------------------------------------

void GB_memoryUsage         // count # allocated blocks and their sizes
(
    int64_t *nallocs,       // # of allocated memory blocks
    size_t *mem_deep,       // # of bytes in blocks owned by this matrix
    size_t *mem_shallow,    // # of bytes in blocks owned by another matrix
    const GrB_Matrix A      // matrix to query
) ;

GB_PUBLIC
void *GB_calloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item,    // sizeof each item
    // output
    size_t *size_allocated, // # of bytes actually allocated
    GB_Context Context
) ;

GB_PUBLIC
void *GB_malloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item,    // sizeof each item
    // output
    size_t *size_allocated  // # of bytes actually allocated
) ;

GB_PUBLIC
void *GB_realloc_memory     // pointer to reallocated block of memory, or
                            // to original block if the realloc failed.
(
    size_t nitems_new,      // new number of items in the object
    size_t size_of_item,    // sizeof each item
    // input/output
    void *p,                // old object to reallocate
    // output
    size_t *size_allocated, // # of bytes actually allocated
    bool *ok,               // true if successful, false otherwise
    GB_Context Context
) ;

GB_PUBLIC
void GB_free_memory         // free memory, bypassing the free_pool
(
    // input/output
    void **p,               // pointer to allocated block of memory to free
    // input
    size_t size_allocated   // # of bytes actually allocated
) ;

GB_PUBLIC
void GB_dealloc_memory      // free memory, return to free_pool or free it
(
    // input/output
    void **p,               // pointer to allocated block of memory to free
    // input
    size_t size_allocated   // # of bytes actually allocated
) ;

GB_PUBLIC
void GB_free_pool_finalize (void) ;

void *GB_xalloc_memory      // return the newly-allocated space
(
    // input
    bool use_calloc,        // if true, use calloc
    bool iso,               // if true, only allocate a single entry
    int64_t n,              // # of entries to allocate if non iso
    size_t type_size,       // size of each entry
    // output
    size_t *size,           // resulting size
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// parallel memcpy and memset
//------------------------------------------------------------------------------

void GB_memcpy                  // parallel memcpy
(
    void *dest,                 // destination
    const void *src,            // source
    size_t n,                   // # of bytes to copy
    int nthreads                // # of threads to use
) ;

void GB_memset                  // parallel memset
(
    void *dest,                 // destination
    const int c,                // value to to set
    size_t n,                   // # of bytes to set
    int nthreads                // # of threads to use
) ;

//------------------------------------------------------------------------------
// malloc/calloc/realloc/free: for permanent contents of GraphBLAS objects
//------------------------------------------------------------------------------

#ifdef GB_MEMDUMP

    #define GB_FREE(p,s) \
    { \
        if (p != NULL && (*(p)) != NULL) \
        { \
            printf ("dealloc (%s, line %d): %p size %lu\n", \
                __FILE__, __LINE__, (*p), s) ; \
        } \
        GB_dealloc_memory ((void **) p, s) ; \
    }

    #define GB_CALLOC(n,type,s) \
        (type *) GB_calloc_memory (n, sizeof (type), s, Context) ; \
        ; printf ("calloc  (%s, line %d): size %lu\n", \
            __FILE__, __LINE__, *(s)) ; \

    #define GB_MALLOC(n,type,s) \
        (type *) GB_malloc_memory (n, sizeof (type), s) ; \
        ; printf ("malloc  (%s, line %d): size %lu\n", \
            __FILE__, __LINE__, *(s)) ; \

    #define GB_REALLOC(p,nnew,type,s,ok,Context) \
        p = (type *) GB_realloc_memory (nnew, sizeof (type), \
            (void *) p, s, ok, Context) ; \
        ; printf ("realloc (%s, line %d): size %lu\n", \
            __FILE__, __LINE__, *(s)) ; \

    #define GB_XALLOC(use_calloc,iso,n,type_size,s) \
        GB_xalloc_memory (use_calloc, iso, n, type_size, s, Context) ; \
        ; printf ("xalloc (%s, line %d): size %lu\n", \
            __FILE__, __LINE__, *(s)) ; \

#else

    #define GB_FREE(p,s) \
        GB_dealloc_memory ((void **) p, s)

    #define GB_CALLOC(n,type,s) \
        (type *) GB_calloc_memory (n, sizeof (type), s, Context)

    #define GB_MALLOC(n,type,s) \
        (type *) GB_malloc_memory (n, sizeof (type), s)

    #define GB_REALLOC(p,nnew,type,s,ok,Context) \
        p = (type *) GB_realloc_memory (nnew, sizeof (type), \
            (void *) p, s, ok, Context)

    #define GB_XALLOC(use_calloc,iso,n,type_size,s) \
        GB_xalloc_memory (use_calloc, iso, n, type_size, s, Context)

#endif

//------------------------------------------------------------------------------
// malloc/calloc/realloc/free: for workspace
//------------------------------------------------------------------------------

// These macros currently do the same thing as the 4 macros above, but that may
// change in the future.  Even if they always do the same thing, it's useful to
// tag the source code for the allocation of workspace differently from the
// allocation of permament space for a GraphBLAS object, such as a GrB_Matrix.

#define GB_CALLOC_WORK(n,type,s) GB_CALLOC(n,type,s)
#define GB_MALLOC_WORK(n,type,s) GB_MALLOC(n,type,s)
#define GB_REALLOC_WORK(p,nnew,type,s,ok,Context) \
             GB_REALLOC(p,nnew,type,s,ok,Context) 
#define GB_FREE_WORK(p,s) GB_FREE(p,s)

#endif

