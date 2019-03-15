//------------------------------------------------------------------------------
// GB_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_init (or GxB_init) must called before any other GraphBLAS operation;
// both rely on this internal function.  If GraphBLAS is used by multiple user
// threads, only one can call GrB_init or GxB_init.

// Result are undefined in multiple user threads simultaneously
// call GrB_init (or GxB_init).

// GrB_finalize must be called as the last GraphBLAS operation.

// GrB_init or GxB_init define the mode that GraphBLAS will use:  blocking or
// non-blocking.  With blocking mode, all operations finish before returning to
// the user application.  With non-blocking mode, operations can be left
// pending, and are computed only when needed.

// GxB_init is the same as GrB_init except that it also defines the
// malloc/calloc/realloc/free functions to use.

// not parallel: this function does O(1) work.

#include "GB.h"

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
// Thread local storage
//------------------------------------------------------------------------------

// Thread local storage is used to to record the details of the last error
// encountered for GrB_error.  If the user application is multi-threaded, each
// thread that calls GraphBLAS needs its own private copy of this report.

#if defined (USER_POSIX_THREADS)
// thread-local storage for POSIX THREADS
pthread_key_t GB_thread_local_key ;

#elif defined (USER_WINDOWS_THREADS)
// for user applications that use Windows threads:
#error Windows threading not yet supported

#elif defined (USER_ANSI_THREADS)
// for user applications that use ANSI C11 threads:
// (this should work per the ANSI C11 specification but is not yet supported)
_Thread_local

#else // USER_OPENMP_THREADS, or USER_NO_THREADS
// OpenMP user threads, or no user threads: this is the default
#endif

#pragma omp threadprivate (GB_thread_local_report)
char GB_thread_local_report [GB_RLEN+1] = "" ;

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

    GB_Context Context      // from GrB_init or GxB_init
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // Don't log the error for GrB_error, since it might not be initialized.

    if (GB_Global.GrB_init_called)
    { 
        // GrB_init can only be called once
        return (GrB_INVALID_VALUE) ;
    }

    if (! (mode == GrB_BLOCKING || mode == GrB_NONBLOCKING))
    {
        // invalid mode
        return (GrB_INVALID_VALUE) ;
    }

    GB_Global.GrB_init_called = true ;

    //--------------------------------------------------------------------------
    // establish malloc/calloc/realloc/free
    //--------------------------------------------------------------------------

    // GrB_init passes in the ANSI C11 malloc/calloc/realloc/free

    GB_Global.malloc_function  = malloc_function  ;
    GB_Global.calloc_function  = calloc_function  ;
    GB_Global.realloc_function = realloc_function ;
    GB_Global.free_function    = free_function    ;

    //--------------------------------------------------------------------------
    // max number of threads
    //--------------------------------------------------------------------------

    // Maximum number of threads for internal parallelization.
    // SuiteSparse:GraphBLAS requires OpenMP to use parallelization within
    // calls to GraphBLAS.  The user application may also call GraphBLAS in
    // parallel, from multiple user threads.  The user threads can use OpenMP,
    // or POSIX pthreads.

    #if defined ( _OPENMP )
    GB_Global.nthreads_max = omp_get_max_threads ( ) ;
    #else
    GB_Global.nthreads_max = 1 ;
    #endif

    //--------------------------------------------------------------------------
    // create the mutex for the critical section, and thread-local storage
    //--------------------------------------------------------------------------

    if (GB_Global.user_multithreaded)
    {

        bool ok = true ;

        #if defined (USER_POSIX_THREADS)
        {
            // initialize the critical section for POSIX pthreads
            int result = pthread_mutex_init (&GB_sync, NULL) ;
            bool ok = (result == 0) ;
            // initialize the key for thread-local storage, allocated in
            // in GB_thread_local_access via GB_Global.calloc_function,
            // and freed by GB_Global.free_function.
            result = pthread_key_create (&GB_thread_local_key, free_function) ;
            ok = ok && (result == 0) ;
        }

        #elif defined (USER_WINDOWS_THREADS)
        {
            // initialize the critical section for Microsoft Windows.
            // This is not yet supported.  See:
            // https://docs.microsoft.com/en-us/windows/desktop/sync
            //  /using-critical-section-objects
            ok = InitializeCriticalSectionAndSpinCount (&GB_sync, 0x00000400) ;
            // also do whatever Windows needs for thread-local-storage
        }

        #elif defined (USER_ANSI_THREADS)
        {
            // initialize the critical section for ANSI C11 threads
            // This should work but is not yet supported.
            ok = (mtx_init (&GB_sync, mtx_plain) == thrd_success) ;
        }

        #else // _OPENMP, USER_OPENMP_THREADS, or USER_NO_THREADS
        {
            // no need to initialize anything for OpenMP
        }
        #endif

        if (!ok) GB_PANIC ;
    }

    GB_thread_local_report [0] = '\0' ;

    //--------------------------------------------------------------------------
    // initialize the global queue
    //--------------------------------------------------------------------------

    // clear the queue of matrices for nonblocking mode and set the mode.  The
    // queue must be protected and can be initialized only once by any thread.

    // clear the queue
    GB_Global.queue_head = NULL ;

    // set the mode: blocking or nonblocking
    GB_Global.mode = mode ;

    //--------------------------------------------------------------------------
    // clear Sauna workspaces
    //--------------------------------------------------------------------------

    for (int t = 0 ; t < GxB_NTHREADS_MAX ; t++)
    { 
        GB_Global.Saunas [t] = NULL ;
        GB_Global.Sauna_in_use [t] = false ;
    }

    //--------------------------------------------------------------------------
    // set the global default format
    //--------------------------------------------------------------------------

    // set the default hypersparsity ratio and CSR/CSC format;  any thread
    // can do this later as well, so there is no race condition danger.

    GB_Global.hyper_ratio = GB_HYPER_DEFAULT ;
    GB_Global.is_csc = (GB_FORMAT_DEFAULT != GxB_BY_ROW) ;

    //--------------------------------------------------------------------------
    // initialize malloc tracking (testing and debugging only)
    //--------------------------------------------------------------------------

    GB_Global_malloc_tracking_set (false) ;
    GB_Global_nmalloc_clear ( ) ;
    GB_Global.malloc_debug = false ;
    GB_Global_malloc_debug_count_set (0) ;
    GB_Global_inuse_clear ( ) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GB_thread_local_access: get pointer to thread-local storage
//------------------------------------------------------------------------------

// This implementation is complete for user threading with POSIX threads,
// OpenMP, and no user threads.  Windows and ANSI C11 threads are not yet
// supported.

// not parallel: this function does O(1) work and is already thread-safe.

char *GB_thread_local_access ( )    // return pointer to thread-local storage
{ 

    //--------------------------------------------------------------------------
    // get pointer to thread-local-storage
    //--------------------------------------------------------------------------

    #if defined (USER_POSIX_THREADS)
    {
        if (GB_Global.user_multithreaded)
        {
            // thread-local storage for POSIX
            char *p = pthread_getspecific (GB_thread_local_key) ;
            bool ok = true ;
            if (p == NULL)
            {
                // first time:  allocate the space for the report
                p = (void *) GB_Global.calloc_function ((GB_RLEN+1),
                    sizeof (char)) ;
                ok = (p != NULL) ;
                ok = ok && (pthread_setspecific (GB_thread_local_key, p) == 0) ;
            }
            // do not attempt to recover from a failure to allocate the space
            return (p) ;
        }
    }
    #elif defined (USER_WINDOWS_THREADS)
    {
        // for user applications that use Windows threads:
        #error "Windows threads not yet supported"
        return (NULL) ;
    }
    #endif

    // USER_OPENMP_THREADS, USER_NO_THREADS, USER_ANSI_THREADS,
    // or USER_POSIX_THREADS but with GB_Global.user_multithreaded false.
    return (GB_thread_local_report) ;
} 

