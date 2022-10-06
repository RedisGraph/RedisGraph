//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/wathen_demo.c: test wathen
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Construct a matrix using the Demo/Source/wathen.c method.
//
//  wathen_demo nx ny method nthreads

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL            \
    GrB_Matrix_free (&A) ;  \

#include "graphblas_demos.h"
#ifdef _OPENMP
#include "omp.h"
#endif

int main (int argc, char **argv)
{
    GrB_Matrix A = NULL ;
    GrB_Info info ;
    OK (GrB_init (GrB_NONBLOCKING)) ;

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    int64_t nx = 10, ny = 10 ;
    int method = 0 ;
    int nthreads ;
    if (argc > 1) nx = strtol (argv [1], NULL, 0) ;
    if (argc > 2) ny = strtol (argv [2], NULL, 0) ;
    if (argc > 3) method = strtol (argv [3], NULL, 0) ;
    if (argc > 4)
    {
        nthreads = strtol (argv [4], NULL, 0) ;
        OK (GxB_Global_Option_set (GxB_GLOBAL_NTHREADS, nthreads)) ;
    }
    OK (GxB_Global_Option_get (GxB_GLOBAL_NTHREADS, &nthreads)) ;

    fprintf (stderr, "Wathen: nx %d ny %d method: %d nthreads: %d ",
        (int) nx, (int) ny, method, nthreads) ;

    //--------------------------------------------------------------------------
    // create a Wathen matrix
    //--------------------------------------------------------------------------

    #ifdef _OPENMP
    double t = omp_get_wtime ( ) ;
    #endif
    OK (wathen (&A, nx, ny, false, method, NULL)) ;
    #ifdef _OPENMP
    t = omp_get_wtime ( ) - t ;
    fprintf (stderr, "time: %g", t) ;
    #endif
    fprintf (stderr, "\n") ;

    OK (GxB_print (A, GxB_SUMMARY)) ;

    FREE_ALL ;

    OK (GrB_finalize ( )) ;
}

