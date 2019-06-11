//------------------------------------------------------------------------------
// bfs_test: read in (or create) a matrix and test BFS
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
// bfs_test s < in > out
// s is the staring node, in is the Matrix Market file, out is the level set.

// TODO clean this up

#include "bfs_test.h"

#define LAGRAPH_FREE_ALL            \
{                                   \
    GrB_free (&AT) ;                \
    GrB_free (&A) ;                 \
    GrB_free (&Abool) ;             \
    GrB_free (&v) ;                 \
    GrB_free (&v2) ;                \
    GrB_free (&v3) ;                \
    GrB_free (&v4) ;                \
    GrB_free (&v5) ;                \
    GrB_free (&v5_tree) ;           \
    GrB_free (&v6) ;                \
    GrB_free (&pi) ;                \
    GrB_free (&v_whole) ;           \
    GrB_free (&pi_whole) ;          \
    GrB_free (&diff) ;              \
}

int main (int argc, char **argv)
{

    GrB_Info info ;
    uint64_t seed = 1 ;

    GrB_Matrix AT = NULL ;
    GrB_Matrix A = NULL ;
    GrB_Matrix Abool = NULL ;
    GrB_Vector v = NULL ;
    GrB_Vector v2 = NULL ;
    GrB_Vector v3 = NULL ;
    GrB_Vector v4 = NULL ;
    GrB_Vector v5 = NULL ;
    GrB_Vector v5_tree = NULL ;
    GrB_Vector v6 = NULL ;
    GrB_Vector v_whole = NULL ;
    GrB_Vector pi = NULL ;
    GrB_Vector pi_whole = NULL ;
    GrB_Vector diff = NULL ;

    LAGRAPH_OK (LAGraph_init ( )) ;

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

    //--------------------------------------------------------------------------
    // get the source node
    //--------------------------------------------------------------------------

    GrB_Index s = 0 ;
    if (argc > 1)
    {
        sscanf (argv [1], "%" PRIu64, &s) ; 
    }

    fprintf (stderr, "\n=========="
        "input graph: nodes: %lu edges: %lu source node: %lu\n", n, nvals, s) ;

    //--------------------------------------------------------------------------
    // run the BFS on node s
    //--------------------------------------------------------------------------

    int ntrials = 1 ;       // increase this to 10, 100, whatever, for more
                            // accurate timing

    // start the timer
    double tic [2] ;
    LAGraph_tic (tic) ;

    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&v) ;
        LAGRAPH_OK (LAGraph_bfs_simple (&v, A, s)) ;
    }

    // stop the timer
    double t1 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "simple    time: %12.6e (sec), rate: %g (1e6 edges/sec)\n",
        t1, 1e-6*((double) nvals) / t1) ;

    //--------------------------------------------------------------------------
    // run the BFS on node s with LAGraph_bfs2 (PUSH)
    //--------------------------------------------------------------------------

    // start the timer
    LAGraph_tic (tic) ;

    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&v2) ;
        LAGRAPH_OK (LAGraph_bfs2 (&v2, A, s, INT32_MAX)) ;
    }

    // stop the timer
    double t2 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "method2   time: %12.6e (sec), rate: %g (1e6 edges/sec)\n",
        t2, 1e-6*((double) nvals) / t2) ;
    fprintf (stderr, "speedup of method2:   %g\n", t1/t2) ;

    LAGRAPH_OK (GrB_assign (v2, v2, NULL, v2, GrB_ALL, n, LAGraph_desc_ooor)) ;

    //--------------------------------------------------------------------------
    // AT = A'
    //--------------------------------------------------------------------------

    LAGraph_tic (tic) ;
    LAGRAPH_OK (GrB_Matrix_new (&AT, GrB_BOOL, ncols, nrows)) ;
    LAGRAPH_OK (GrB_transpose (AT, NULL, NULL, A, NULL)) ;
    double transpose_time = LAGraph_toc (tic) ;
    fprintf (stderr, "transpose time: %g\n", transpose_time) ;

    //--------------------------------------------------------------------------
    // now the BFS on node s using push-pull instead
    //--------------------------------------------------------------------------

    LAGraph_tic (tic) ;

    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&v3) ;
        LAGRAPH_OK (LAGraph_bfs_pushpull_old (&v3, A, AT, s, INT32_MAX)) ;
    }

    double t3 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "push/pull old:  %12.6e (sec), "
        " rate: %g (1e6 edges/sec)\n",
        t3, 1e-6*((double) nvals) / t3) ;

    LAGRAPH_OK (GrB_assign (v3, v3, NULL, v3, GrB_ALL, n, LAGraph_desc_ooor)) ;

    //--------------------------------------------------------------------------
    // now the BFS on node s using push-pull (BEST) instead
    //--------------------------------------------------------------------------

    int nthreads_max = 1 ;
    #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
    GxB_get (GxB_NTHREADS, &nthreads_max) ;
    fprintf (stderr, "\n====== parallel:\n") ;
    #endif

    double t5 [nthreads_max+1] ;

    fprintf (stderr, "\n") ;

    for (int nthreads = 1 ; nthreads <= nthreads_max ; nthreads++)
    {
        #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
        GxB_set (GxB_NTHREADS, nthreads) ;
        #endif

        LAGraph_tic (tic) ;
        for (int trial = 0 ; trial < ntrials ; trial++)
        {
            GrB_free (&v5) ;
            LAGRAPH_OK (LAGraph_bfs_pushpull (&v5, NULL, A, AT, s, 0, false)) ;
        }
        t5 [nthreads] = LAGraph_toc (tic) / ntrials ;
        fprintf (stderr, "nthreads %d push/pull best: %12.6e (sec), "
            " rate: %g (1e6 edges/sec)\n",
            nthreads, t5 [nthreads], 1e-6*((double) nvals) / t5 [nthreads]) ;
        if (nthreads > 1) fprintf (stderr, "speedup %g\n",
            t5 [1] / t5 [nthreads]) ;
    }

    #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
    // restore default
    GxB_set (GxB_NTHREADS, nthreads_max) ;
    #endif

    fprintf (stderr, "\n") ;

    LAGRAPH_OK (GrB_assign (v5, v5, NULL, v5, GrB_ALL, n, LAGraph_desc_ooor)) ;

    //--------------------------------------------------------------------------
    // now the BFS on node s using push-pull (BEST) instead
    //--------------------------------------------------------------------------

    double t5_tree [nthreads_max+1] ;

    fprintf (stderr, "parallel (with tree):\n") ;

    for (int nthreads = 1 ; nthreads <= nthreads_max ; nthreads++)
    {
        #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
        GxB_set (GxB_NTHREADS, nthreads) ;
        #endif

        LAGraph_tic (tic) ;
        for (int trial = 0 ; trial < ntrials ; trial++)
        {
            GrB_free (&v5_tree) ;
            GrB_free (&pi) ;
            LAGRAPH_OK (LAGraph_bfs_pushpull (&v5_tree, &pi, A, AT, s, 0,
                false)) ;
        }
        t5_tree [nthreads] = LAGraph_toc (tic) / ntrials ;
        fprintf (stderr, "nthreads %d push/pull TREE: %12.6e (sec), "
            " rate: %g (1e6 edges/sec)\n",
            nthreads, t5_tree [nthreads],
            1e-6*((double) nvals) / t5_tree [nthreads]) ;
        if (nthreads > 1) fprintf (stderr, "speedup %g\n",
            t5_tree [1] / t5_tree [nthreads]) ;
    }

    #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
    // restore default
    GxB_set (GxB_NTHREADS, nthreads_max) ;
    #endif

    fprintf (stderr, "\n") ;

    LAGRAPH_OK (GrB_assign (v5_tree, v5_tree, NULL, v5_tree, GrB_ALL,
        n, LAGraph_desc_ooor)) ;

    // TODO: check the tree
    GrB_free (&pi) ;

    //--------------------------------------------------------------------------
    // now the BFS on node s using push-pull (BEST) instead
    //--------------------------------------------------------------------------

    fprintf (stderr, "v starts sparse: (nthread %d)\n", nthreads_max) ;
    LAGraph_tic (tic) ;
    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&v6) ;
        LAGRAPH_OK (LAGraph_bfs_pushpull (&v6, NULL, A, AT, s, 0, true)) ;
    }
    double t6 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "pushpull sparse %12.6e (sec), rate: %g (1e6 edges/sec)\n",
        t6, 1e-6*((double) nvals) / t6) ;

    int32_t maxlevel = 0 ;

#if 0

    //--------------------------------------------------------------------------
    // BFS on the whole graph
    //--------------------------------------------------------------------------

    fprintf (stderr, "\nwhole graph, no tree:\n") ;
    LAGraph_tic (tic) ;
    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&v_whole) ;
        LAGRAPH_OK (LAGraph_bfs_pushpull (&v_whole, NULL, A, AT, -1, 0, false));
    }
    double tw = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "whole graph %12.6e (sec), rate: %g (1e6 edges/sec)\n"
        "NOTE: the other BFS's are single-source; this is the whole graph)\n",
        tw, 1e-6*((double) nvals) / tw) ;

    // find the max level
    LAGRAPH_OK (GrB_reduce (&maxlevel, NULL, LAGraph_MAX_INT32_MONOID, v_whole,
        NULL));
    fprintf (stderr, "number of levels: %d (for whole graph)\n", maxlevel) ;

    //--------------------------------------------------------------------------
    // BFS on the whole graph, also compute the tree
    //--------------------------------------------------------------------------

    fprintf (stderr, "\nwhole graph, with tree:\n") ;
    LAGraph_tic (tic) ;
    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&v_whole) ;
        GrB_free (&pi_whole) ;
        LAGRAPH_OK (LAGraph_bfs_pushpull (&v_whole, &pi_whole,
            A, AT, -1, 0, false)) ;
    }
    double tpi = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "whole graph %12.6e (sec), rate: %g (1e6 edges/sec)"
        "with tree\n",
        tpi, 1e-6*((double) nvals) / tpi) ;
#endif

    //--------------------------------------------------------------------------
    // now the BFS on node s using PULL (only) instead
    //--------------------------------------------------------------------------

    // slow!!!
/*
    fprintf (stderr, "starting pull-only BFS ... please wait, this is slow!\n");
    LAGraph_tic (tic) ;
    LAGRAPH_OK (LAGraph_bfs_pull (&v4, A, AT, s, INT32_MAX)) ;
    double t4 = LAGraph_toc (tic) ;
    fprintf (stderr, "pull      time: %12.6e (sec), rate: %g (1e6 edges/sec)\n",
        t4, 1e-6*((double) nvals) / t4) ;
    fprintf (stderr, "speedup of push/pull: %g (normally slow!)\n", t1/t4) ;

    LAGRAPH_OK (GrB_assign (v4, v4, NULL, v4, GrB_ALL, n, LAGraph_desc_ooor)) ;
*/

    //--------------------------------------------------------------------------
    // check results
    //--------------------------------------------------------------------------

    bool isequal = false, ok = true ;

    // find the max level
    LAGRAPH_OK (GrB_reduce (&maxlevel, NULL, LAGraph_MAX_INT32_MONOID, v,
        NULL));
    fprintf (stderr, "number of levels: %d (for s = %lu, single-source)\n",
        maxlevel, s) ;

    // find the number of nodes visited
    GrB_Index nv = 0 ;
    LAGRAPH_OK (GrB_Vector_nvals (&nv, v)) ;
    fprintf (stderr, "# nodes visited (for single-source): %lu out of %lu"
        " (%g %% of the graph)\n", nv, n,
        100. * (double) nv / (double) n) ;

    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, v, v2, NULL)) ;
    if (!isequal)
    {
        fprintf (stderr, "ERROR! simple and method2   differ\n") ;
        ok = false ;
    }

/*
    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, v, v4, NULL)) ;
    if (!isequal)
    {
        fprintf (stderr, "ERROR! simple and PULL   differ\n") ;
    }
*/

    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, v, v5_tree, NULL)) ;
    if (!isequal)
    {
        fprintf (stderr, "ERROR! simple and TREE   differ\n") ;
        ok = false ;
    }

    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, v, v5, NULL)) ;
    if (!isequal)
    {
        fprintf (stderr, "ERROR! simple and best   differ\n") ;
        ok = false ;
    }

    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, v, v3, NULL)) ;
    if (!isequal)
    {
        // GxB_fprint (v,  2, stderr) ;
        // GxB_fprint (v3, 2, stderr) ;
        fprintf (stderr, "ERROR! simple and push-pull differ\n") ;
        ok = false ;
    }

    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, v, v6, NULL)) ;
    if (!isequal)
    {
        // GxB_fprint (v,  2, stderr) ;
        // GxB_fprint (v6, 2, stderr) ;
        fprintf (stderr, "ERROR! simple and push-pull (sparse) differ\n") ;
        ok = false ;
    }

    #if 0
    // diff = v5 - v
    LAGRAPH_OK (GrB_Vector_new (&diff, GrB_INT32, n)) ;
    LAGRAPH_OK (GrB_eWiseAdd (diff, NULL, NULL, GrB_MINUS_INT32, v5, v, NULL)) ;
    // err = or (diff)
    bool err ;
    LAGRAPH_OK (GrB_reduce (&err, NULL, LAGraph_LOR_MONOID, diff, NULL)) ;
    if (err)
    {
        fprintf (stderr, "diff (v5-v):\n") ;
        // GxB_fprint (diff, 2, stderr) ;
        // GxB_fprint (v5, 2, stderr) ;
        // GxB_fprint (v,  2, stderr) ;
        fprintf (stderr, "ERROR! simple and best differ\n") ;
    }
    else
    {
        fprintf (stderr, "diff is the same\n") ;
    }
    #endif

    //--------------------------------------------------------------------------
    // write the result to stdout (check them outside of this main program)
    //--------------------------------------------------------------------------

    for (int64_t i = 0 ; i < n ; i++)
    {
        // if the entry v(i) is not present, x is unmodified, so '0' is printed
        int64_t x = 0 ;
        LAGRAPH_OK (GrB_Vector_extractElement (&x, v, i)) ;
        printf ("%" PRIu64 "\n", x) ;
    }

    //--------------------------------------------------------------------------
    // free all workspace and finish
    //--------------------------------------------------------------------------

    LAGRAPH_FREE_ALL ;
    LAGRAPH_OK (LAGraph_finalize ( )) ;
    fprintf (stderr, "bfs_test: ") ;
    if (ok)
    {
        fprintf (stderr, "all tests passed\n") ;
    }
    else
    {
        fprintf (stderr, "TEST FAILURE\n") ;
    }
    fprintf (stderr,
    "------------------------------------------------------------\n\n") ;
    return (GrB_SUCCESS) ;
}

