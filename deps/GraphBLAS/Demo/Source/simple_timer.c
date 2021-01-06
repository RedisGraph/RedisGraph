//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/simple_timer.c: a timer for performance measurements
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// simple_timer:  a portable timer for accurate performance measurements

#define GB_LIBRARY
#include "simple_timer.h"

//------------------------------------------------------------------------------
// simple_tic: return the current wallclock time in high resolution
//------------------------------------------------------------------------------

GB_PUBLIC
void simple_tic         // returns current time in seconds and nanoseconds
(
    double tic [2]      // tic [0]: seconds, tic [1]: nanoseconds
)
{

    #if defined ( _OPENMP )

        // OpenMP is available; use the OpenMP timer function
        tic [0] = omp_get_wtime ( ) ;
        tic [1] = 0 ;

    #elif defined ( __linux__ ) || defined ( __GNU__ )

        // Linux has a very low resolution clock() function, so use the high
        // resolution clock_gettime instead.  May require -lrt
        struct timespec t ;
        clock_gettime (CLOCK_MONOTONIC, &t) ;
        tic [0] = (double) t.tv_sec ;
        tic [1] = (double) t.tv_nsec ;

    #elif defined ( __MACH__ ) && defined ( __APPLE__ )

        // otherwise, on the Mac, use the MACH timer
        clock_serv_t cclock ;
        mach_timespec_t t ;
        host_get_clock_service (mach_host_self ( ), SYSTEM_CLOCK, &cclock) ;
        clock_get_time (cclock, &t) ;
        mach_port_deallocate (mach_task_self ( ), cclock) ;
        tic [0] = (double) t.tv_sec;
        tic [1] = (double) t.tv_nsec;

    #else

        // The ANSI C11 clock() function is used instead.  This gives the
        // processor time, not the wallclock time, and it might have low
        // resolution.  It returns the time since some unspecified fixed time
        // in the past, as a clock_t integer.  The clock ticks per second are
        // given by CLOCKS_PER_SEC.  In Mac OSX this is a very high resolution
        // clock, and clock ( ) is faster than clock_get_time (...) ;
        clock_t t = clock ( ) ;
        tic [0] = ((double) t) / ((double) CLOCKS_PER_SEC) ;
        tic [1] = 0 ;

    #endif

}

//------------------------------------------------------------------------------
// simple_toc: return the time since the last simple_tic
//------------------------------------------------------------------------------

GB_PUBLIC
double simple_toc           // returns time since last simple_tic
(
    const double tic [2]    // tic from last call to simple_tic
)
{
    double toc [2] ;
    simple_tic (toc) ;
    return ((toc [0] - tic [0]) + 1e-9 * (toc [1] - tic [1])) ;
}

