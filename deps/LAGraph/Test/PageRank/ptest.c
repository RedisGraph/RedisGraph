//------------------------------------------------------------------------------
// ptest: read in (or create) a matrix and test PageRank
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

// Contributed by Tim Davis, Texas A&M

// usage:
// ptest < in > out

#include "LAGraph.h"

#define LAGRAPH_FREE_ALL                        \
{                                               \
    if (P != NULL) { free (P) ; P = NULL ; }    \
    GrB_free (&A) ;                             \
    GrB_free (&Abool) ;                         \
}

int main ( )
{

    GrB_Info info ;
    GrB_Matrix A = NULL ;
    GrB_Matrix Abool = NULL ;
    LAGraph_PageRank *P = NULL ;

    LAGRAPH_OK (LAGraph_init ( )) ;

    int nthreads_max = 1 ;
    #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
    GxB_get (GxB_NTHREADS, &nthreads_max) ;
    GxB_set (GxB_NTHREADS, 1) ;
    #endif

    //--------------------------------------------------------------------------
    // read in a matrix from a file and convert to boolean
    //--------------------------------------------------------------------------

    // read in the file in Matrix Market format
    LAGRAPH_OK (LAGraph_mmread (&A, stdin)) ;
    // GxB_fprint (A, GxB_COMPLETE, stderr) ;
    // LAGraph_mmwrite (A, stderr) ;

    // convert to boolean, pattern-only
    LAGRAPH_OK (LAGraph_pattern (&Abool, A)) ;
    // LAGraph_mmwrite (Abool, stderr) ;
    GrB_free (&A) ;
    A = Abool ;
    Abool = NULL ;
    // GxB_fprint (A, GxB_COMPLETE, stderr) ;

    // finish any pending computations
    GrB_Index nvals ;
    GrB_Matrix_nvals (&nvals, A) ;

    //--------------------------------------------------------------------------
    // get the size of the problem.
    //--------------------------------------------------------------------------

    GrB_Index nrows, ncols ;
    LAGRAPH_OK (GrB_Matrix_nrows (&nrows, A)) ;
    LAGRAPH_OK (GrB_Matrix_ncols (&ncols, A)) ;
    GrB_Index n = nrows ;

    // GxB_fprint (A, GxB_COMPLETE, stderr) ;

    // LAGRAPH_OK (GrB_Matrix_setElement (A, 0, 0, n-1)) ;     // hack

    fprintf (stderr, "\n=========="
        "input graph: nodes: %"PRIu64" edges: %"PRIu64"\n", n, nvals) ;

    //--------------------------------------------------------------------------
    // compute the pagerank
    //--------------------------------------------------------------------------

    int ntrials = 1 ;       // increase this to 10, 100, whatever, for more
                            // accurate timing
    
    double tol = 1e-5 ;
    int iters, itermax = 100 ;

    int nthread_list [7] = {1, 2, 4, 8, 16, 20, 40} ;

    // for (int nthreads = 1 ; nthreads <= nthreads_max ; nthreads *= 2)
    for (int kk = 0 ; kk < 7 ; kk++)
    {
        int nthreads = nthread_list [kk] ;

        #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
        GxB_set (GxB_NTHREADS, nthreads) ;
        #endif

        // start the timer
        double tic [2] ;
        LAGraph_tic (tic) ;

        for (int trial = 0 ; trial < ntrials ; trial++)
        {
            if (P != NULL) { free (P) ; P = NULL ; }
            LAGRAPH_OK (LAGraph_pagerank (&P, A, itermax, tol, &iters)) ;
        }

        // stop the timer
        double t1 = LAGraph_toc (tic) / ntrials ;
        fprintf (stderr, "pagerank  time: %12.6e (sec), "
            "rate: %g (1e6 edges/sec) iters: %d threads: %d\n",
            t1, 1e-6*((double) nvals) / t1, iters, nthreads) ;
    }

    //--------------------------------------------------------------------------
    // print results
    //--------------------------------------------------------------------------

    for (int64_t k = 0 ; k < n ; k++)
    {
        printf ("%" PRIu64 " %g\n", P [k].page, P [k].pagerank) ;
    }

    //--------------------------------------------------------------------------
    // free all workspace and finish
    //--------------------------------------------------------------------------

    LAGRAPH_FREE_ALL ;
    LAGRAPH_OK (LAGraph_finalize ( )) ;
    return (GrB_SUCCESS) ;
}

