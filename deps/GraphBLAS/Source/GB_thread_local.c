//------------------------------------------------------------------------------
// GB_thread_local: manage thread-local storage
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This implementation is complete for user threading with POSIX threads,
// OpenMP, and no user threads.  Windows and ANSI C11 threads are not yet
// supported.

// Thread local storage is used to to record the details of the last error
// encountered for GrB_error.  If the user application is multi-threaded, each
// thread that calls GraphBLAS needs its own private copy of this report.

// These two functions are defined here:

//      GB_thread_local_init:       initialize thread-local storage
//      GB_thread_local_get:        get pointer to thread-local storage

// They access the following global or thread-local variables, which are
// defined and accessible only in this file:

//      GB_thread_local_key:        for POSIX threads only
//      GB_thread_local_report:     for OpenMP and ANSI C11 threads only

#include "GB_thread_local.h"

#if defined ( USER_POSIX_THREADS )
// thread-local storage for POSIX THREADS
pthread_key_t GB_thread_local_key ;

#elif defined ( USER_WINDOWS_THREADS )
// for user applications that use Windows threads:
#error "Windows threading not yet supported"

#elif defined ( USER_ANSI_THREADS )
// for user applications that use ANSI C11 threads:
// (this should work per the ANSI C11 specification but is not yet supported)
_Thread_local
char GB_thread_local_report [GB_RLEN+1] = "" ;

#else // USER_OPENMP_THREADS, or USER_NO_THREADS
// OpenMP user threads, or no user threads: this is the default
char GB_thread_local_report [GB_RLEN+1] = "" ;
#pragma omp threadprivate(GB_thread_local_report)
#endif

//------------------------------------------------------------------------------
// GB_thread_local_init: initialize thread-local storage
//------------------------------------------------------------------------------

bool GB_thread_local_init
(
    void (* free_function) (void *)     // used for POSIX threads only
)
{ 
    #if defined ( USER_POSIX_THREADS )
    {
        // initialize the key for thread-local storage, allocated in
        // GB_thread_local_get via GB_Global_calloc_function, and freed by
        // GB_Global_free_function.
        return (pthread_key_create (&GB_thread_local_key, free_function) == 0) ;
    }
    #elif defined ( USER_WINDOWS_THREADS )
    {
        // do whatever Windows needs for thread-local-storage
        #error "Windows threads not yet supported"
        return (false) ;        // not yet implemented
    }
    #else
    {
        // _OPENMP, USER_OPENMP_THREADS, USER_ANSI_THREADS, or USER_NO_THREADS
        GB_thread_local_report [0] = '\0' ;
        return (true) ;
    }
    #endif
}

//------------------------------------------------------------------------------
// GB_thread_local_get: get pointer to thread-local storage
//------------------------------------------------------------------------------

char *GB_thread_local_get (void)        // get pointer to thread-local storage
{ 
    #if defined ( USER_POSIX_THREADS )
    {
        // thread-local storage for POSIX
        char *p = pthread_getspecific (GB_thread_local_key) ;
        if (p == NULL)
        {
            // first time:  allocate the space for the report
            p = (void *) GB_Global_calloc_function ((GB_RLEN+1), sizeof (char));
            if (p != NULL) pthread_setspecific (GB_thread_local_key, p) ;
        }
        // do not attempt to recover from a failure to allocate the space;
        // just return the NULL pointer on failure.  The caller will catch it
        // and return a GrB_PANIC to its caller.
        return (p) ;
    }
    #elif defined ( USER_WINDOWS_THREADS )
    {
        // for user applications that use Windows threads:
        #error "Windows threads not yet supported"
        return (NULL) ;
    }
    #else // USER_OPENMP_THREADS, USER_NO_THREADS, USER_ANSI_THREADS,
    {
        return (GB_thread_local_report) ;
    }
    #endif
}

