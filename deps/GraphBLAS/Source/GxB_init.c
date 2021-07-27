//------------------------------------------------------------------------------
// GxB_init: initialize GraphBLAS and declare malloc/calloc/realloc/free to use
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GrB_init (or GxB_init) must called before any other GraphBLAS operation.
// GrB_finalize must be called as the last GraphBLAS operation.  GxB_init is
// identical to GrB_init, except that it allows the user application to define
// the malloc/calloc/realloc/free functions that SuiteSparse:GraphBLAS will
// use.  The functions cannot be modified once GraphBLAS starts.

// The calloc and realloc function pointers are optional and can be NULL.  If
// calloc is NULL, it is not used, and malloc/memset are used instead.  If
// realloc is NULL, it is not used, and malloc/memcpy/free are used instead.

// Examples:
//
// To use GraphBLAS with the ANSI C11 functions (or to another library
// linked in that replaces them): 
//
//      // either use:
//      GrB_init (mode) ;
//      // or use this (but not both):
//      GxB_init (mode, malloc, calloc, realloc, free, true) ;
//
// To use GraphBLAS from within a mexFunction:
//
//      #include "mex.h"
//      GxB_init (mode, mxMalloc, mxCalloc, mxRealloc, mxFree, false) ;
//
// To use the C interface to the Intel TBB scalable allocators:
//
//      #include "tbb/scalable_allocator.h"
//      GxB_init (mode, scalable_malloc, scalable_calloc, scalable_realloc,
//          scalable_free, true) ;
//
// To use CUDA, do not use this function.  Instead use the following:
//
//      GxB_cuda_init (mode) ;
//
//      // All GraphBLAS objects are allocated with GxB_cuda_malloc, and
//      // GxB_cuda_free.  This function can still be used if CUDA is not
//      // available at compile-time; in this case, it acts just like GrB_init
//      // (mode).  The memory allocators GxB_cuda_malloc, etc., just call the
//      // ANSI C malloc/free functions.
//
// To use user-provided malloc and free functions, but not calloc/realloc:
//
//      GxB_init (mode, my_malloc, NULL, NULL, my_free, true) ;

#include "GB.h"

GrB_Info GxB_init           // start up GraphBLAS and also define malloc, etc
(
    GrB_Mode mode,          // blocking or non-blocking mode

    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),         // required
    void * (* user_calloc_function  ) (size_t, size_t), // no longer used
    void * (* user_realloc_function ) (void *, size_t), // optional, can be NULL
    void   (* user_free_function    ) (void *),         // required
    bool user_malloc_is_thread_safe
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_CONTEXT ("GxB_init (mode, malloc, calloc, realloc, free, thread_safe)") ;
    if (user_malloc_function == NULL || user_free_function == NULL)
    { 
        // only malloc and free are required.  calloc and/or realloc may be
        // NULL
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    return (GB_init
        (mode,                          // blocking or non-blocking mode
        user_malloc_function,           // user-defined malloc, required
        NULL,                           // user-defined calloc, ignored
        user_realloc_function,          // user-defined realloc, may be NULL
        user_free_function,             // user-defined free, required
        user_malloc_is_thread_safe,     // true if all functions are thread-safe
        false,                          // do not use CUDA
        Context)) ;
}

