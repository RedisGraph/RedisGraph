//------------------------------------------------------------------------------
// GB_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GrB_init, GxB_init, or GxB_cuda_init must called before any other GraphBLAS
// operation; all three rely on this internal function.  If GraphBLAS is used
// by multiple user threads, only one can call GrB_init, GxB_init or
// GxB_cuda_init.

// Result are undefined if multiple user threads simultaneously
// call GrB_init, GxB_init, or GxB_cuda_init.

// GrB_finalize must be called as the last GraphBLAS operation.

// GrB_init, GxB_init, or GxB_cuda_init define the mode that GraphBLAS will
// use:  blocking or non-blocking.  With blocking mode, all operations finish
// before returning to the user application.  With non-blocking mode,
// operations can be left pending, and are computed only when needed.

// GxB_init is the same as GrB_init except that it also defines the
// malloc/calloc/realloc/free functions to use.

// GxB_cuda_init is the same as GrB_init, except that it passes in
// caller_is_GxB_cuda_init as true to this function.  GxB_init and GrB_init
// both pass this flag in as false.

#include "GB.h"

//------------------------------------------------------------------------------
// GB_init
//------------------------------------------------------------------------------

GrB_Info GB_init            // start up GraphBLAS
(
    const GrB_Mode mode,    // blocking or non-blocking mode

    // pointers to memory management functions.  Must be non-NULL.
    void * (* malloc_function  ) (size_t),
    void * (* calloc_function  ) (size_t, size_t),
    void * (* realloc_function ) (void *, size_t),
    void   (* free_function    ) (void *),
    bool malloc_is_thread_safe,

    bool caller_is_GxB_cuda_init,       // true for GxB_cuda_init only

    GB_Context Context      // from GrB_init or GxB_init
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (GB_Global_GrB_init_called_get ( ))
    { 
        // GrB_init can only be called once
        return (GrB_INVALID_VALUE) ;
    }

    GB_Global_GrB_init_called_set (true) ;

    if (! (mode == GrB_BLOCKING || mode == GrB_NONBLOCKING))
    { 
        // invalid mode
        return (GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // establish malloc/calloc/realloc/free
    //--------------------------------------------------------------------------

    // GrB_init passes in the ANSI C11 malloc/calloc/realloc/free
    // GxB_cuda_init passes in NULL pointers; they are now defined below.

    if (caller_is_GxB_cuda_init)
    {
        #if defined ( GBCUDA )
        // CUDA is available at compile time, and requested at run time via
        // GxB_cuda_init.  Use CUDA unified memory management functions.
        malloc_function = GxB_cuda_malloc ;
        calloc_function = GxB_cuda_calloc ;
        realloc_function = NULL ;
        free_function = GxB_cuda_free ;
        #else
        // CUDA not available at compile time.  Use ANSI C memory managment
        // functions instead, even though the caller is GxB_cuda_init.
        // No GPUs will be used.
        malloc_function = malloc ;
        calloc_function = calloc ;
        realloc_function = realloc ;
        free_function = free ;
        #endif
    }

    GB_Global_malloc_function_set  (malloc_function ) ;
    GB_Global_calloc_function_set  (calloc_function ) ;
    GB_Global_realloc_function_set (realloc_function) ;
    GB_Global_free_function_set    (free_function   ) ;
    GB_Global_malloc_is_thread_safe_set (malloc_is_thread_safe) ;

    // #include "GB_init_mkl_template.c"

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

    // set the default hyper_switch and CSR/CSC format;  any thread
    // can do this later as well, so there is no race condition danger.

    GB_Global_hyper_switch_set (GB_HYPER_SWITCH_DEFAULT) ;
    GB_Global_bitmap_switch_default ( ) ;
    GB_Global_is_csc_set (GB_FORMAT_DEFAULT != GxB_BY_ROW) ;

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

    //--------------------------------------------------------------------------
    // development use only
    //--------------------------------------------------------------------------

    GB_Global_timing_clear_all ( ) ;

    //--------------------------------------------------------------------------
    // CUDA initializations
    //--------------------------------------------------------------------------

    // If CUDA exists (#define GBCUDA) and if the caller is GxB_cuda_init, then
    // query the system for the # of GPUs available, their memory sizes, SM
    // counts, and other capabilities.  Unified Memory support is assumed.
    // Then warmup each GPU.

    #if defined ( GBCUDA )
    if (caller_is_GxB_cuda_init)
    {
        // query the system for the # of GPUs
        // TODO for GPU: make this a function in the CUDA folder
        GB_Global_gpu_control_set (GxB_DEFAULT) ;
        if (!GB_Global_gpu_count_set (true)) return (GrB_PANIC) ;
        int gpu_count = GB_Global_gpu_count_get ( ) ;
        for (int device = 0 ; device < 1 ; device++) // TODO for GPU: gpu_count
        {
            // query the GPU and then warm it up
            if (!GB_Global_gpu_device_properties_get (device))
            {
                return (GrB_PANIC) ;
            }
            if (!GB_cuda_warmup (device))
            {
                return (GrB_PANIC) ;
            }
        }
        // make GPU 0 the default device
        GB_cuda_set_device( 0 );

        // also check for jit cache, pre-load library of common kernels ...
    }
    else
    #endif
    {
        // CUDA not available at compile-time, or available but not requested.
        GB_Global_gpu_control_set (GxB_GPU_NEVER) ;
        GB_Global_gpu_count_set (0) ;
    }

    GB_Global_gpu_chunk_set (GxB_DEFAULT) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

