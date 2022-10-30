//------------------------------------------------------------------------------
// GB_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GrB_init or GxB_init must called before any other GraphBLAS
// operation; all three rely on this internal function.  If GraphBLAS is used
// by multiple user threads, only one can call GrB_init or GxB_init.

// Result are undefined if multiple user threads simultaneously call GrB_init
// or GxB_init.

// Per the spec, GrB_finalize must be called as the last GraphBLAS operation.
// Not even GrB_Matrix_free can be safely called after GrB_finalize.  In the
// current version of SuiteSparse:GraphBLAS, GrB_finalize does nothing, but in
// future versions it may do critical work such as freeing a memory pool.

// GrB_init or GxB_init define the mode that GraphBLAS will use:  blocking or
// non-blocking.  With blocking mode, all operations finish before returning to
// the user application.  With non-blocking mode, operations can be left
// pending, and are computed only when needed.

// GxB_init is the same as GrB_init except that it also defines the
// malloc/realloc/free functions to use.

// The realloc function pointer is optional and can be NULL.  If realloc is
// NULL, it is not used, and malloc/memcpy/free are used instead.

#include "GB.h"

//------------------------------------------------------------------------------
// GB_init
//------------------------------------------------------------------------------

GrB_Info GB_init            // start up GraphBLAS
(
    const GrB_Mode mode,    // blocking or non-blocking mode

    // pointers to memory management functions.
    void * (* malloc_function  ) (size_t),          // required
    void * (* realloc_function ) (void *, size_t),  // optional, can be NULL
    void   (* free_function    ) (void *),          // required

    GB_Context Context      // from GrB_init or GxB_init
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #ifdef GBCUDA_DEV
    printf ("\n==== GBCUDA_DEV enabled: for CUDA development only! ====\n") ;
    #endif

    if (GB_Global_GrB_init_called_get ( ))
    { 
        // GrB_init can only be called once
        return (GrB_INVALID_VALUE) ;
    }

    GB_Global_GrB_init_called_set (true) ;

    if (mode < GrB_NONBLOCKING || mode > GxB_BLOCKING_GPU)
    { 
        // invalid mode
        return (GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // query hardware features for future use
    //--------------------------------------------------------------------------

    GB_Global_cpu_features_query ( ) ;

    //--------------------------------------------------------------------------
    // establish malloc/realloc/free
    //--------------------------------------------------------------------------

    // GrB_init passes in the ANSI C11 malloc/realloc/free

    GB_Global_malloc_function_set  (malloc_function ) ; // cannot be NULL
    GB_Global_realloc_function_set (realloc_function) ; // ok if NULL
    GB_Global_free_function_set    (free_function   ) ; // cannot be NULL
    GB_Global_malloc_is_thread_safe_set (true) ; // malloc must be thread-safe
    GB_Global_memtable_clear ( ) ;
    GB_Global_free_pool_init (true) ;

    //--------------------------------------------------------------------------
    // max number of threads
    //--------------------------------------------------------------------------

    // Maximum number of threads for internal parallelization.
    // SuiteSparse:GraphBLAS requires OpenMP to use parallelization within
    // calls to GraphBLAS.  The user application may also call GraphBLAS in
    // parallel, from multiple user threads.  The user threads can use
    // any threading library; this has no effect on GraphBLAS.

    GB_Global_nthreads_max_set (GB_Global_omp_get_max_threads ( )) ;
    GB_Global_chunk_set (GB_CHUNK_DEFAULT) ;

    //--------------------------------------------------------------------------
    // initialize the blocking/nonblocking mode
    //--------------------------------------------------------------------------

    // set the mode: blocking or nonblocking
    GB_Global_mode_set (mode) ;

    //--------------------------------------------------------------------------
    // set the global default format
    //--------------------------------------------------------------------------

    // set the default hyper_switch and the default format (by-row);  any thread
    // can do this later as well, so there is no race condition danger.

    GB_Global_hyper_switch_set (GB_HYPER_SWITCH_DEFAULT) ;
    GB_Global_bitmap_switch_default ( ) ;
    GB_Global_is_csc_set (false) ;

    //--------------------------------------------------------------------------
    // initialize malloc tracking (testing and debugging only)
    //--------------------------------------------------------------------------

    GB_Global_malloc_tracking_set (false) ;
    GB_Global_nmalloc_clear ( ) ;
    GB_Global_malloc_debug_set (false) ;
    GB_Global_malloc_debug_count_set (0) ;

    //--------------------------------------------------------------------------
    // diagnostic output
    //--------------------------------------------------------------------------

    GB_Global_burble_set (false) ;
    GB_Global_printf_set (NULL) ;
    GB_Global_flush_set (NULL) ;

    //--------------------------------------------------------------------------
    // development use only
    //--------------------------------------------------------------------------

    GB_Global_timing_clear_all ( ) ;

    //--------------------------------------------------------------------------
    // CUDA initializations
    //--------------------------------------------------------------------------

    GrB_Info info = GrB_SUCCESS ;
    #if defined ( GBCUDA )
    if (mode == GxB_BLOCKING_GPU || mode == GxB_NONBLOCKING_GPU)
    {
        // initialize the GPUs
        info = GB_cuda_init ( ) ;
    }
    else
    #endif
    { 
        // CUDA not available at compile-time, or not requested at run time
        GB_Global_gpu_control_set (GxB_GPU_NEVER) ;
        GB_Global_gpu_count_set (0) ;
        GB_Global_gpu_chunk_set (GxB_DEFAULT) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (info) ;
}

