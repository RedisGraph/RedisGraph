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

// Determines how many milliseconds are in a second.
#define MILLISECONDS_IN_SECOND 1000
// Determines how many nanoseconds are in a millisecond.
#define NANOSECONDS_IN_MILLISECOND 1000000

// An alias for the type of the timer.
#define TIMER_TYPE double[2]
// Defines a timer variable. An easy way to define a variable of a timer named
// "name" which doesn't care about the precise timer type (double[2]), so it
// also can be easily changed to whichever else.
#define TIMER_DEFINE(name) double name[2]
// An alias to assign for the timer.
#define TIMER_ASSIGN(lhs, rhs) lhs[0] = rhs[0]; lhs[1] = rhs[1]
// (Re)starts the timer.
#define TIMER_RESTART(name) simple_tic(name)
// Returns the seconds (double) counted by the timer.
#define TIMER_GET_SECONDS(name) name[0]
// Returns the seconds (uint64_t) counted by the timer.
#define TIMER_GET_FULL_SECONDS(name) (uint64_t)TIMER_GET_SECONDS(name)
// Returns the nanoseconds (double) counted by the timer.
#define TIMER_GET_NANOSECONDS(name) name[1]
// Returns the nanoseconds (uint64_t) counted by the timer.
#define TIMER_GET_FULL_NANOSECONDS(name) (uint64_t)TIMER_GET_NANOSECONDS(name)
// Returns the time counted in full milliseconds.
// TODO rounding?
#define TIMER_GET_MILLISECONDS(name) \
    (uint64_t)(TIMER_GET_SECONDS(name) * MILLISECONDS_IN_SECOND) + \
    (uint64_t)(TIMER_GET_NANOSECONDS(name) / NANOSECONDS_IN_MILLISECOND)
// An alias to return the time in seconds passed since the previous call.
#define TIMER_GET_ELAPSED(name) simple_toc(name)
// An alias to return the time in milliseconds passed since the previous call.
#define TIMER_GET_ELAPSED_MILLISECONDS(name) \
    (uint64_t)(TIMER_GET_ELAPSED(name) * MILLISECONDS_IN_SECOND)
// Defines a timer variable and starts the timer immediately.
#define TIMER_DEFINE_AND_START(name) TIMER_DEFINE(name); TIMER_RESTART(name)

void simple_tic         // returns current time in seconds and nanoseconds
(
    double tic [2]      // tic [0]: seconds, tic [1]: nanoseconds
) ;

double simple_toc           // returns time since last simple_tic
(
    const double tic [2]    // tic from last call to simple_tic
) ;

#endif
