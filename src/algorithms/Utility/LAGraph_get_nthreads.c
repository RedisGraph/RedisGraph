//------------------------------------------------------------------------------
// LAGraph_get_nthreads: get the # of threads to use
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// LAGraph_get_nthreads: get # of threads that will be used by GraphBLAS.

// SuiteSparse:GraphBLAS has a mechanism that controls the number of threads
// that will be used in each operation.  This value is set by GxB_set and
// retreived by GxB_get.  It may be less than the value returned
// omp_get_max_threads.

// GrB_init (and GxB_init) keeps track of this as a global setting, and
// initializes this value to omp_get_max_threads.  If you want to test the
// performance of a LAGraph or GraphBLAS function (when using SuiteSparse:
// GraphBLAS), you can modify this value with LAGraph_set_nthreads, and you
// can retrieve it with LAGraph_get_threads.  Here is an example:

/*
    LAGraph_init ( ) ;  // start GraphBLAS and LAGraph

    // get the current # of threads to use. 
    int nthreads_max = LAGraph_get_nthreads ( ) ;
    int nthreads_max2 = omp_get_max_nthreads ( ) ;

    printf ("omp max: %d, to use in LAGraph: %d\n", nthreads_max,
        nthreads_max2) ;
    assert (nthreads_max == nthreads_max2) ;

    // do some computation using nthreads_max
    double tic [2], t ;
    LAGraph_tic (tic) ;
    LAGraph_lcc ( ... ) ;
    double t = LAGraph_toc (tic) ;
    printf ("time to solve the problem using %d threads: %g (seconds)\n',
        nthreads_max, t) ;

    // OK, now let's try it again with different #'s of threads:
    for (int k = nthreads_max ; k > 0 ; k--)
    {
        LAGraph_set_nthreads (k) ;

        LAGraph_tic (tic) ;
        LAGraph_lcc ( ... ) ;
        t = LAGraph_toc (tic) ;

        printf ("time to solve the problem using %d threads: %g (seconds)\n',
            nthreads_max, t) ;
    }

    int now_nthreads = LAGraph_get_nthreads ( ) ;
    printf ("Now the nthread setting is %d\n", now_nthreads) ;
    assert (now_nthreads == 1) ;

    // do something with one thread
    LAGraph_tic (tic) ;
    LAGraph_lcc ( ... ) ;
    t = LAGraph_toc (tic) ;

    printf ("hey, %g sec is slow! (because I told LAGraph to use %d thread)\n",
        now_nthreads) ;

    // So reset it back to the max
    LAGraph_set_nthreads (nthreads_max) ;

    LAGraph_tic (tic) ;
    LAGraph_lcc ( ... ) ;
    t = LAGraph_toc (tic) ;

    now_nthreads = LAGraph_get_nthreads ( ) ;
    printf ("oh, %g sec is much faster if I use all the threads (%d)\n",
        now_nthreads) ;
    assert (now_nthreads == nthreads_max) ;

*/

// NOTE:  the GraphBLAS API does not have a GxB_set or GxB_get to control the #
// of threads that GraphBLAS should use.  Those are SuiteSparse:GraphBLAS
// extensions.  The above code will work in any GraphBLAS library; it will just
// likely run with all the CPU threads and/or GPU cores and/or MPI nodes it can
// use (assuming // it's a multi-threaded library or GPU code, or whatever).

// To allow this function to compile when SuiteSparse:GraphBLAS is not being
// used, the use of the SuiteSparse:GraphBLAS-specific function GxB_get is
// protected by an #ifdef.

// contributed by Tim Davis, Texas A&M

// See also LAGraph_set_nthreads

#include "LAGraph_internal.h"

int LAGraph_get_nthreads    // returns # threads to use, 1 if unknown
(
    void
)
{

    int nthreads = 1 ;

    #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
    GrB_Info info = GxB_get (GxB_NTHREADS, &nthreads) ;
    if (info != GrB_SUCCESS)
    {
        fprintf (stderr, "LAGraph error:\n[%d]\nFile: %s Line: %d\n",
            info, __FILE__, __LINE__) ;
        return (-9999) ;
    }
    #elif defined ( _OPENMP )
    nthreads = omp_get_max_threads ( ) ;
    #else
    // nothing to do ...
    #endif

    return (nthreads) ;
}

