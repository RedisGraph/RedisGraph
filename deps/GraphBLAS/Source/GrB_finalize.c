//------------------------------------------------------------------------------
// GrB_finalize: finalize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_finalize must be called as the last GraphBLAS function, per the
// GraphBLAS C API Specification.  Only one user thread can call this
// function.  Results are undefined if more than one thread calls this
// function at the same time.

// not parallel: this function does O(1) work

#include "GB.h"

GrB_Info GrB_finalize ( )
{ 

    //--------------------------------------------------------------------------
    // free all workspace
    //--------------------------------------------------------------------------

    for (int Sauna_id = 0 ; Sauna_id < GxB_NTHREADS_MAX ; Sauna_id++)
    { 
        GB_Sauna_free (Sauna_id) ;
    }

    //--------------------------------------------------------------------------
    // destroy the queue
    //--------------------------------------------------------------------------

    if (GB_Global.user_multithreaded)
    {

        #if defined (USER_POSIX_THREADS)
        {
            // delete the critical section for POSIX pthreads
            pthread_mutex_destroy (&GB_sync) ;
            // thread-local storage will be deleted when the user thread
            // terminates, using GB_Global.free_function passed to
            // pthread_key_create in GB_init.
        }

        #elif defined (USER_WINDOWS_THREADS)
        {
            // delete the critical section for Microsoft Windows.
            // This is not yet supported.  See:
            // https://docs.microsoft.com/en-us/windows/desktop/sync
            //  /using-critical-section-objects
            DeleteCriticalSection (&GB_sync) ;
            // also do whatever Windows needs to free thread-local-storage
        }

        #elif defined (USER_ANSI_THREADS)
        {
            // delete the critical section for ANSI C11 threads
            // This should work but is not yet supported.
            mtx_destroy (&GB_sync) ;
            // thread-local storage is statically allocated and does not need
            // to be freed.
        }

        #else // USER_OPENMP_THREADS or USER_NO_THREADS
        {
            // no need to finalize anything for OpenMP or for no user threads
            ;
        }
        #endif

    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

