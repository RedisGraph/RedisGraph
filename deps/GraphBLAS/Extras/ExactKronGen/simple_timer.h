/* -------------------------------------------------------------------------- */
/* GraphBLAS/Demo/simple_timer.h: a timer for performance measurements        */
/* -------------------------------------------------------------------------- */

/* SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.    */
/* http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.        */

/* -------------------------------------------------------------------------- */

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

#define _POSIX_C_SOURCE 200809L
#include <time.h>

#if defined ( __linux__ )
#include <sys/time.h>
#endif

#if defined ( _OPENMP )
#include <omp.h>
#endif

void simple_tic         /* returns current time in seconds and nanoseconds */
(
    double tic [2]      /* tic [0]: seconds, tic [1]: nanoseconds */
) ;


double simple_toc           /* returns time since last simple_tic */
(
    const double tic [2]    /* tic from last call to simple_tic */
) ;

