//------------------------------------------------------------------------------
// GB_queue_create:  create the global matrix queue and thread-local storage
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

bool GB_queue_create ( )        // create the queue and thread-local storage
{

    //--------------------------------------------------------------------------
    // create the mutex for the critical section, and thread-local storage
    //--------------------------------------------------------------------------

    bool ok = true ;

    #if defined (USER_POSIX_THREADS)
    {
        // initialize the critical section for POSIX pthreads
        ok = pthread_mutex_init (&(GB_Global.sync), NULL) == 0 ;
        // initialize the key for thread-local storage, using GB_CALLOC to
        // allocate it (in GB_thread_local_access) and GB_FREE to free it.
        ok = ok & (pthread_key_create (&GB_thread_local_report, GB_FREE) == 0) ;
    }

    #elif defined (USER_WINDOWS_THREADS)
    {
        // initialize the critical section for Microsoft Windows.
        // This is not yet supported.  See:
        // https://docs.microsoft.com/en-us/windows/desktop/sync
        //  /using-critical-section-objects
        ok = InitializeCriticalSectionAndSpinCount (&(GB_Global.sync),
            0x00000400) ;
        // also do whatever Windows needs for thread-local-storage
    }

    #elif defined (USER_ANSI_THREADS)
    {
        // initialize the critical section for ANSI C11 threads
        // This should work but is not yet supported.
        ok = (mtx_init (&(GB_Global.sync), mtx_plain) == thrd_success) ;
        GB_thread_local_report [0] = '\0' ;
    }

    #else // _OPENMP, USER_OPENMP_THREADS, or USER_NO_THREADS
    {
        // no need to initialize anything for OpenMP
        GB_thread_local_report [0] = '\0' ;
    }
    #endif

    return (ok) ;
}

