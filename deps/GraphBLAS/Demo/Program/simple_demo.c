//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/simple_demo.c: tests simple_rand and simple_timer
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

/*
    A simple test that illustrates the use of simple_rand and simple_tic/toc.
    This test does not require ANSI C11, nor GraphBLAS.  It only tests the
    simple_* Demo functions.  The output of this test should look like the
    following.  The random numbers should be the same.  On a Mac OSX system:

        time to call simple_tic 1 million times: 0.367798
        time to generate 10 million random numbers: 0.10633
        first 10 random numbers:
            0.513871
            0.175726
            0.308634
            0.534532
            0.947630
            0.171728
            0.702231
            0.226417
            0.494766
            0.124699

    The clock_gettime function in Linux is even faster (0.02 seconds to call
    simple_tic 1 million times).  The random numbers are identical, as desired.
*/

#include "simple_rand.h"
#include "simple_timer.h"
#include <stdio.h>
#include <stdlib.h>

#define LEN 10000000

int main (void)
{
    double *x ;
    double tic [2], t ;
    int i ;

    fprintf (stderr, "simple_demo:\n") ;
    double n = ((double) LEN) / 1e6 ;

    // calloc the space for more accurate timing
    x = (double *) calloc (LEN, sizeof (double)) ;
    if (x == NULL)
    {
        fprintf (stderr, "simple_demo: out of memory\n") ;
        exit (1) ;
    }

    // do lots of tics
    simple_tic (tic) ;
    for (i = 0 ; i < LEN/10 ; i++)
    {
        double tic2 [2] ;
        simple_tic (tic2) ;
    }
    t = simple_toc (tic) ;
    printf ("time to call simple_tic %g million times: %g\n", n/10, t) ;

    // generate random numbers
    simple_tic (tic) ;
    for (i = 0 ; i < LEN ; i++)
    {
        x [i] = simple_rand_x ( ) ;
    }
    t = simple_toc (tic) ;

    // report the result
    printf ("time to generate %g million random numbers: %g\n", n, t) ;
    fprintf (stderr, "time to generate %g million random numbers: %g\n\n",
        n, t) ;

    // these should be the same on any system and any compiler
    printf ("first 10 random numbers:\n") ;
    for (i = 0 ; i < 10 ; i++)
    {
        printf ("%12.6f\n", x [i]) ;
    }

    // generate random uint64_t numbers
    double t1 ;

    simple_tic (tic) ;
    for (i = 0 ; i < LEN ; i++)
    {
        simple_rand_i ( ) ;
    }
    t1 = simple_toc (tic) ;

    printf ("time to generate %g million random uint64: %g\n", n, t1) ;
    fprintf (stderr, "time to generate %g million random uint64: %g\n\n",
        n, t1) ;

    free (x) ;
}

