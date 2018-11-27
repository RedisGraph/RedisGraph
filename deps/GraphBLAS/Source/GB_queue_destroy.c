//------------------------------------------------------------------------------
// GB_queue_destroy: destroy the global matrix queue
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

bool GB_queue_destroy ( )       // destroy the queue
{ 

    bool ok = true ;

    #if defined (USER_POSIX_THREADS)
    {
        // delete the critical section for POSIX pthreads
        ok = (pthread_mutex_destroy (&(GB_Global.sync)) == 0) ;
        // thread-local storage will be deleted when the user thread
        // terminates, using the destructor function (GB_FREE) passed to
        // pthread_key_create in GrB_init.
    }

    #elif defined (USER_WINDOWS_THREADS)
    {
        // delete the critical section for Microsoft Windows.
        // This is not yet supported.  See:
        // https://docs.microsoft.com/en-us/windows/desktop/sync
        //  /using-critical-section-objects
        DeleteCriticalSection (&(GB_Global.sync)) ;
        // also do whatever Windows needs to free thread-local-storage
    }

    #elif defined (USER_ANSI_THREADS)
    {
        // delete the critical section for ANSI C11 threads
        // This should work but is not yet supported.
        mtx_destroy (&(GB_Global.sync)) ;
        // thread-local storage is statically allocated and does not need
        // to be freed.
    }

    #else // USER_OPENMP_THREADS or USER_NO_THREADS
    {
        // no need to finalize anything for OpenMP or for no user threads
        ;
    }
    #endif

    return (ok) ;
}

