//------------------------------------------------------------------------------
// GrB_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_init must called before any other GraphBLAS operation.  GrB_finalize
// must be called as the last GraphBLAS operation.

// GrB_init defines the mode that GraphBLAS will use:  blocking or
// non-blocking.  With blocking mode, all operations finish before returning to
// the user application.  With non-blocking mode, operations can be left
// pending, and are computed only when needed.

// The GrB_wait function forces all pending operations to complete.  Blocking
// mode is as if the GrB_wait operation is called whenever a GraphBLAS
// operation returns to the user.

// The non-blocking mode can have side effects if user-defined functions have
// side effects or if they rely on global variables, which are not under the
// control of GraphBLAS.  Suppose the user creates a user-defined operator that
// accesses a global variable.  That operator is then used in a GraphBLAS
// operation, which is left pending.  If the user then changes the global
// variable, the pending operations will be eventually computed with this
// different value.

// Worse yet, a user-defined operator can be freed before it is needed to
// finish a pending operation.  To avoid this, call GrB_wait before modifying
// any global variables relied upon by user-defined operators and before
// freeing any user-defined types, operators, monoids, or semirings.

#include "GB.h"

//------------------------------------------------------------------------------
// Thread local storage
//------------------------------------------------------------------------------

// Thread local storage is used to to record the details of the last error
// encountered for GrB_error.  If the user application is multi-threaded, each
// thread that calls GraphBLAS needs its own private copy of this report.

#if defined (USER_POSIX_THREADS)
// thread-local storage for POSIX THREADS
pthread_key_t GB_thread_local_report ;

#elif defined (USER_WINDOWS_THREADS)
// for user applications that use Windows threads:
#error Windows threading not yet supported

#elif defined (USER_ANSI_THREADS)
// for user applications that use ANSI C11 threads:
// (this should work per the ANSI C11 specification but is not yet supported)
_Thread_local char GB_thread_local_report [GB_RLEN+1] = "" ;

#else // USER_OPENMP_THREADS, or USER_NO_THREADS
// OpenMP user threads, or no user threads: this is the default
#pragma omp threadprivate (GB_thread_local_report)
char GB_thread_local_report [GB_RLEN+1] = "" ;
#endif

//------------------------------------------------------------------------------
// All Global storage is declared and initialized here
//------------------------------------------------------------------------------

// If the user creates threads that work on GraphBLAS matrices, then all of
// those threads must share the same matrix queue, and the same mode.

GB_Global_struct GB_Global =
{

    // queued matrices with work to do
    .queue_head = NULL,         // pointer to first queued matrix

    // GraphBLAS mode
    .mode = GrB_NONBLOCKING,    // default is nonblocking

    // initialization flag
    .GrB_init_called = false,   // GrB_init has not yet been called

    // default format
    .hyper_ratio = GB_HYPER_DEFAULT,
    .is_csc = (GB_FORMAT_DEFAULT != GxB_BY_ROW)     // default is GxB_BY_ROW

    #ifdef GB_MALLOC_TRACKING
    // malloc tracking, for testing, statistics, and debugging only
    , .nmalloc = 0                // memory block counter
    , .malloc_debug = false       // do not test memory handling
    , .malloc_debug_count = 0     // counter for testing memory handling
    , .inuse = 0                  // memory space current in use
    , .maxused = 0                // high water memory usage
    #endif

} ;

//------------------------------------------------------------------------------
// GrB_init
//------------------------------------------------------------------------------

// If GraphBLAS is used by multiple user threads, only one can call GrB_init.

GrB_Info GrB_init           // start up GraphBLAS
(
    const GrB_Mode mode     // blocking or non-blocking mode
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_init (mode)") ;

    //--------------------------------------------------------------------------
    // create the global queue and thread-local storage
    //--------------------------------------------------------------------------

    GB_CRITICAL (GB_queue_create ( )) ;

    //--------------------------------------------------------------------------
    // initialize the global queue
    //--------------------------------------------------------------------------

    // Only one thread should initialize these settings.  If multiple threads
    // call GrB_init, only the first thread does this work.

    if (! (mode == GrB_BLOCKING || mode == GrB_NONBLOCKING))
    { 
        // mode is invalid; also report the error for GrB_error.
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "Unknown mode: %d; must be %d (GrB_NONBLOCKING) or %d"
            " (GrB_BLOCKING)", (int) mode, (int) GrB_NONBLOCKING,
            (int) GrB_BLOCKING))) ;
    }

    bool I_was_first ;

    GB_CRITICAL (GB_queue_init (mode, &I_was_first)) ;

    if (! I_was_first)
    { 
        return (GB_ERROR (GrB_INVALID_VALUE, (GB_LOG,
            "GrB_init must not be called twice"))) ;
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

    #ifdef GB_MALLOC_TRACKING
    // malloc tracking.  This is only for statistics and development.
    {
        GB_Global.nmalloc = 0 ;
        GB_Global.malloc_debug = false ;
        GB_Global.malloc_debug_count = 0 ;
        GB_Global.inuse = 0 ;
        GB_Global.maxused = 0 ;
    }
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

