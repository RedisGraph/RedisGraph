//------------------------------------------------------------------------------
// GB_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_init (or GxB_init) must called before any other GraphBLAS operation;
// both rely on this internal function.  If GraphBLAS is used by multiple user
// threads, only one can call GrB_init or GxB_init.

// Result are undefined if multiple user threads simultaneously
// call GrB_init (or GxB_init).

// GrB_finalize must be called as the last GraphBLAS operation.

// GrB_init or GxB_init define the mode that GraphBLAS will use:  blocking or
// non-blocking.  With blocking mode, all operations finish before returning to
// the user application.  With non-blocking mode, operations can be left
// pending, and are computed only when needed.

// GxB_init is the same as GrB_init except that it also defines the
// malloc/calloc/realloc/free functions to use.

#include "GB_thread_local.h"

//------------------------------------------------------------------------------
// critical section for user threads
//------------------------------------------------------------------------------

// User-level threads may call GraphBLAS in parallel, so the access to the
// global queue for GrB_wait must be protected by a critical section.  The
// critical section method should match the user threading model.

#if defined (USER_POSIX_THREADS)
// for user applications that use POSIX pthreads
pthread_mutex_t GB_sync ;

#elif defined (USER_WINDOWS_THREADS)
// for user applications that use Windows threads (not yet supported)
CRITICAL_SECTION GB_sync ; 

#elif defined (USER_ANSI_THREADS)
// for user applications that use ANSI C11 threads (not yet supported)
mtx_t GB_sync ;

#else // USER_OPENMP_THREADS, or USER_NO_THREADS
// nothing to do for OpenMP, or for no user threading

#endif

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

    GB_Context Context      // from GrB_init or GxB_init
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // Do not log the error for GrB_error, since it might not be initialized.

    if (GB_Global_GrB_init_called_get ( ))
    { 
        // GrB_init can only be called once
        return (GrB_PANIC) ;
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

    GB_Global_malloc_function_set  (malloc_function ) ;
    GB_Global_calloc_function_set  (calloc_function ) ;
    GB_Global_realloc_function_set (realloc_function) ;
    GB_Global_free_function_set    (free_function   ) ;
    GB_Global_malloc_is_thread_safe_set (malloc_is_thread_safe) ;

    //--------------------------------------------------------------------------
    // max number of threads
    //--------------------------------------------------------------------------

    // Maximum number of threads for internal parallelization.
    // SuiteSparse:GraphBLAS requires OpenMP to use parallelization within
    // calls to GraphBLAS.  The user application may also call GraphBLAS in
    // parallel, from multiple user threads.  The user threads can use OpenMP,
    // or POSIX pthreads.

    GB_Global_nthreads_max_set (GB_Global_omp_get_max_threads ( )) ;
    GB_Global_chunk_set (GB_CHUNK_DEFAULT) ;

    //--------------------------------------------------------------------------
    // initialize thread-local storage
    //--------------------------------------------------------------------------

    if (!GB_thread_local_init (free_function)) GB_PANIC ;

    //--------------------------------------------------------------------------
    // create the mutex for the critical section
    //--------------------------------------------------------------------------

    #if defined (USER_POSIX_THREADS)
    {
        // initialize the critical section for POSIX pthreads
        bool ok = (pthread_mutex_init (&GB_sync, NULL) == 0) ;
        if (!ok) GB_PANIC ;
    }

    #elif defined (USER_WINDOWS_THREADS)
    {
        // initialize the critical section for Microsoft Windows.
        // This is not yet supported.  See:
        // https://docs.microsoft.com/en-us/windows/desktop/sync
        //  /using-critical-section-objects
        bool ok = InitializeCriticalSectionAndSpinCount (&GB_sync, 0x00000400) ;
        // also do whatever Windows needs for thread-local-storage
        if (!ok) GB_PANIC ;
    }

    #elif defined (USER_ANSI_THREADS)
    {
        // initialize the critical section for ANSI C11 threads
        // This should work but is not yet supported.
        bool ok = (mtx_init (&GB_sync, mtx_plain) == thrd_success) ;
        if (!ok) GB_PANIC ;
    }

    #else // _OPENMP, USER_OPENMP_THREADS, or USER_NO_THREADS
    { 
        // no need to initialize anything for OpenMP
        ;
    }
    #endif

    //--------------------------------------------------------------------------
    // initialize the global queue
    //--------------------------------------------------------------------------

    // clear the queue of matrices for nonblocking mode and set the mode.  The
    // queue must be protected and can be initialized only once by any thread.

    // clear the queue
    GB_Global_queue_head_set (NULL) ;

    // set the mode: blocking or nonblocking
    GB_Global_mode_set (mode) ;

    //--------------------------------------------------------------------------
    // set the global default format
    //--------------------------------------------------------------------------

    // set the default hypersparsity ratio and CSR/CSC format;  any thread
    // can do this later as well, so there is no race condition danger.

    GB_Global_hyper_ratio_set (GB_HYPER_DEFAULT) ;
    GB_Global_is_csc_set (GB_FORMAT_DEFAULT != GxB_BY_ROW) ;

    //--------------------------------------------------------------------------
    // initialize malloc tracking (testing and debugging only)
    //--------------------------------------------------------------------------

    GB_Global_malloc_tracking_set (false) ;
    GB_Global_nmalloc_clear ( ) ;
    GB_Global_malloc_debug_set (false) ;
    GB_Global_malloc_debug_count_set (0) ;
    GB_Global_inuse_clear ( ) ;

    //--------------------------------------------------------------------------
    // development use only; controls diagnostic output
    //--------------------------------------------------------------------------

    GB_Global_burble_set (false) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

