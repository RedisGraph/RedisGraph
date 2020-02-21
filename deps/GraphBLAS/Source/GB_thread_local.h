//------------------------------------------------------------------------------
// GB_thread_local.h: definitions for thread local storage
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Thread local storage is created by GrB_init or GxB_init (via GB_init),
// and then accessed by the error logging mechanism (GB_error), and the
// error reporting function GrB_error.

#ifndef GB_THREAD_LOCAL_H
#define GB_THREAD_LOCAL_H

#include "GB.h"

#if defined ( USER_POSIX_THREADS )
// thread-local storage for POSIX THREADS
extern pthread_key_t GB_thread_local_key ;

#elif defined ( USER_WINDOWS_THREADS )
// for user applications that use Windows threads:
#error "Windows threads not yet supported"

#elif defined ( USER_ANSI_THREADS )
// for user applications that use ANSI C11 threads:
// (this should work per the ANSI C11 specification but is not yet supported)
_Thread_local
extern char GB_thread_local_report [GB_RLEN+1] ;

#else
// _OPENMP, USER_OPENMP_THREADS, or USER_NO_THREADS
// This is the default.

extern char GB_thread_local_report [GB_RLEN+1] ;
#endif

bool GB_thread_local_init               // intialize thread-local storage
(
    void (* free_function) (void *)
) ;

char *GB_thread_local_get (void) ;      // get pointer to thread-local storage

#endif
