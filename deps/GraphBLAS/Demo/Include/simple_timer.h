//------------------------------------------------------------------------------
// GraphBLAS/Demo/Include/simple_timer.h: a timer for performance measurements
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

/*
   There is no method that works on all operating systems for finding the
   current time with high resolution that is suitable for performance
   measurements.  The simple_timer.c and simple_timer.h functions provide a
   portable alternative.

   simple_tic (tic) ; gets the current time and saves it in tic [0..1].

   t = simple_toc (tic) ; returns the time in seconds since the last call to
   simple_toc, as a single double value.

   Usage:

        #include "simple_timer.h"
        double tic [2], r, s, t ;

        simple_tic (tic) ;          // start the timer
        // do some work A
        t = simple_toc (tic) ;      // t is time for work A, in seconds
        // do some work B
        s = simple_toc (tic) ;      // s is time for work A and B, in seconds

        simple_tic (tic) ;          // restart the timer
        // do some work C
        r = simple_toc (tic) ;      // r is time for work C, in seconds
*/

#ifndef SIMPLE_TIMER_H
#define SIMPLE_TIMER_H

#ifndef GB_PUBLIC
// Exporting/importing symbols for Microsoft Visual Studio
#if ( _MSC_VER && !__INTEL_COMPILER )
#ifdef GB_LIBRARY
// compiling SuiteSparse:GraphBLAS itself, exporting symbols to user apps
#define GB_PUBLIC extern __declspec ( dllexport )
#else
// compiling the user application, importing symbols from SuiteSparse:GraphBLAS
#define GB_PUBLIC extern __declspec ( dllimport )
#endif
#else
// for other compilers
#define GB_PUBLIC extern
#endif
#endif

#include <time.h>

//------------------------------------------------------------------------------
// decide which timer to use
//------------------------------------------------------------------------------

#if defined ( _OPENMP )

    // if OpenMP is available, use omp_get_wtime
    #include <omp.h>

#elif defined ( __linux__ ) || defined ( __GNU__ )

    // otherwise, on Linux/GNU, use clock_gettime. May require -lrt
    #include <sys/time.h>

#elif defined ( __MACH__ ) && defined ( __APPLE__ )

    // otherwise, on the Mac, use the MACH timer
    #include <mach/clock.h>
    #include <mach/mach.h>

#else

    // Finally, the ANSI C11 clock() function is used if no other timer
    // is available.

#endif

//------------------------------------------------------------------------------

GB_PUBLIC
void simple_tic         // returns current time in seconds and nanoseconds
(
    double tic [2]      // tic [0]: seconds, tic [1]: nanoseconds
) ;

GB_PUBLIC
double simple_toc           // returns time since last simple_tic
(
    const double tic [2]    // tic from last call to simple_tic
) ;

#endif

