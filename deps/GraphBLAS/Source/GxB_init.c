//------------------------------------------------------------------------------
// GxB_init: initialize GraphBLAS and declare malloc/calloc/realloc/free to use
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
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
//      GxB_init (mode, malloc, calloc, realloc, free) ;
//
// To use GraphBLAS from within a mexFunction:
//
//      #include "mex.h"
//      GxB_init (mode, mxMalloc, mxCalloc, mxRealloc, mxFree) ;
//
// To use the C interface to the Intel TBB scalable allocators:
//
//      #include "tbb/scalable_allocator.h"
//      GxB_init (mode, scalable_malloc, scalable_calloc, scalable_realloc,
//          scalable_free) ;
//
// To use CUDA and its RMM memory manager:
//
//      GxB_init (mode, rmm_malloc, rmm_calloc, rmm_realloc, rmm_free) ;
//
//          mode is GrB_BLOCKING or GrB_NONBLOCKING

#if for_comments_only
compute_system = rmm_wrap_initialize (mode, initpoolsize, maxpoolsize) ;

    create RMM instance
    query the GPU(s) available, set their context
    compute_system: holds 4 RMM contexts, 4 GPUs, how big they are ...

p = rmm_wrap_malloc (42) ;  // needs the GPUs to be warmed up
...

    // option:
    GxB_init (GrB_NONBLOCKING, rmm_wrap_malloc, rmm_wrap_calloc,
        rmm_wrap_realloc, rmm_wrap_free) ;

    // use GrB just on the CPU cores
    GrB_Matrix_new (&C, ...)
    GrB_mxm (...)

    GxB_set (GxB_CUDA_SYSTEM_CONTEXT, compute_system) ;   // use the GPUs ...
    GxB_set (GxB_NTHREDS, 4) ;  // use 4 cpu threads

    GxB_get (GxB_CUDA_NGPUS, &ngpus)

    // use GrB just on the GPU 2
    GxB_set (GxB_CUDA_SET_DEVICE, 2) ;
    GrB_mxm (C, ...)
    GxB_set (C, GxB_SPARSITY, GxB_SPARSE + GxB_HYPERSPARE) ;
    GxB_Matrix_Option_set

    GrB_mxm (C, ...)

    ...
    GxB_set (GxB_CUDA, true) ;      // 0 seconds, GPUs already warmed up
    ...
    GxB_set (GxB_CUDA, false) ;
    ...
    GxB_set (GxB_CUDA, true) ;      // 0 seconds
    GxB_set (GxB_GPUS, [0 2]) ;
    ...

GrB_finalize ( ) ;
rmm_wrap_free (p) ;
rmm_wrap_finalize ( ) ;
#endif

//
// To use user-provided malloc and free functions, but not calloc/realloc:
//
//      GxB_init (mode, my_malloc, NULL, NULL, my_free) ;

#include "GB.h"

GrB_Info GxB_init           // start up GraphBLAS and also define malloc, etc
(
    GrB_Mode mode,          // blocking or non-blocking mode

    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),         // required
    void * (* user_calloc_function  ) (size_t, size_t), // no longer used
    void * (* user_realloc_function ) (void *, size_t), // optional, can be NULL
    void   (* user_free_function    ) (void *)          // required
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_CONTEXT ("GxB_init (mode, malloc, calloc, realloc, free)") ;
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
        user_realloc_function,          // user-defined realloc, may be NULL
        user_free_function,             // user-defined free, required
        Context)) ;
}

