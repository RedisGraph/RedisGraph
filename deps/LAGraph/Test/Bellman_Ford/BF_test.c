//------------------------------------------------------------------------------
// Test/Bellman_Ford/BF_test.c:  test for LAGraph_BF_*
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

// Contributed by Jinhao Chen, Texas A&M

// usage:
// BF_test s < in > out
// s is the staring node, which should be in the range of [0, nrows),
// in is the Matrix Market file, out is the level set.

#include "BF_test.h"

#define LAGRAPH_FREE_ALL            \
{                                   \
    GrB_free (&A) ;                 \
    GrB_free (&AT) ;                \
    free(I);                        \
    free(J);                        \
    free(W);                        \
    free(d);                        \
    free(pi);                       \
    GrB_free (&d1) ;                \
    GrB_free (&pi1) ;               \
    GrB_free (&h1) ;                \
    GrB_free (&d2) ;                \
    GrB_free (&pi2) ;               \
    GrB_free (&h2) ;                \
    GrB_free (&d3) ;                \
    GrB_free (&d4) ;                \
}

int main (int argc, char **argv)
{
    GrB_Info info;
    double tic [2] ;
    GrB_Matrix A = NULL ;
    GrB_Matrix AT = NULL ;
    GrB_Index *I = NULL, *J = NULL; // for col/row indices of entries from A
    double *W = NULL;
    double *d = NULL;
    int64_t *pi = NULL;
    GrB_Vector d1 = NULL ;
    GrB_Vector pi1 = NULL ;
    GrB_Vector h1 = NULL ;
    GrB_Vector d2 = NULL ;
    GrB_Vector pi2 = NULL ;
    GrB_Vector h2 = NULL ;
    GrB_Vector d3 = NULL ;
    GrB_Vector d4 = NULL ;

    LAGRAPH_OK (LAGraph_init ( )) ;

    //--------------------------------------------------------------------------
    // read in a matrix from a file and convert to boolean
    //--------------------------------------------------------------------------

    // read in the file in Matrix Market format
    LAGRAPH_OK (LAGraph_mmread (&A, stdin)) ;
    // GxB_fprint (A, GxB_COMPLETE, stderr) ;
    // LAGraph_mmwrite (A, stderr) ;

    //--------------------------------------------------------------------------
    // get the size of the problem.
    //--------------------------------------------------------------------------

    GrB_Index nvals ;
    GrB_Matrix_nvals (&nvals, A) ;
    GrB_Index nrows, ncols ;
    LAGRAPH_OK (GrB_Matrix_nrows (&nrows, A)) ;
    LAGRAPH_OK (GrB_Matrix_ncols (&ncols, A)) ;
    GrB_Index n = nrows ;
    
    I = LAGraph_malloc (nvals, sizeof(GrB_Index)) ; 
    J = LAGraph_malloc (nvals, sizeof(GrB_Index)) ; 
    W = LAGraph_malloc (nvals, sizeof(double)) ;

    LAGRAPH_OK (GrB_Matrix_extractTuples_FP64(I, J, W, &nvals, A));

    //--------------------------------------------------------------------------
    // set the diagonal to 0
    //--------------------------------------------------------------------------
    for (GrB_Index i = 0; i < n; i++)
    {
        LAGRAPH_OK (GrB_Matrix_setElement_FP64 (A, 0, i, i));
    }

    //--------------------------------------------------------------------------
    // AT = A'
    //--------------------------------------------------------------------------

    LAGraph_tic (tic) ;
    LAGRAPH_OK (GrB_Matrix_new (&AT, GrB_FP64, ncols, nrows)) ;
    LAGRAPH_OK (GrB_transpose (AT, NULL, NULL, A, NULL)) ;
    double transpose_time = LAGraph_toc (tic) ;
    fprintf (stderr, "transpose     time: %g\n", transpose_time) ;
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
    // run the LAGraph_BF_full on node s
    //--------------------------------------------------------------------------

    int ntrials = 1 ;       // increase this to 10, 100, whatever, for more
                            // accurate timing

    // start the timer
    LAGraph_tic (tic) ;

    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&d1) ;
        GrB_free (&pi1) ;
        LAGRAPH_OK (LAGraph_BF_full (&d1, &pi1, &h1, A, s)) ;
    }

    // stop the timer
    double t1 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "BF_full       time: %12.6e (sec), rate:"
        " %g (1e6 edges/sec)\n", t1, 1e-6*((double) nvals) / t1) ;

    //--------------------------------------------------------------------------
    // run the BF on node s with LAGraph_BF_basic
    //--------------------------------------------------------------------------

    // start the timer
    LAGraph_tic (tic) ;

    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&d3) ;
        LAGRAPH_OK (LAGraph_BF_basic (&d3, A, s)) ;
    }

    // stop the timer
    double t2 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "BF_basic      time: %12.6e (sec), rate:"
        " %g (1e6 edges/sec)\n", t2, 1e-6*((double) nvals) / t2) ;
    fprintf (stderr, "speedup of BF_basic:       %g\n", t1/t2) ;

    //--------------------------------------------------------------------------
    // run the BF on node s with LAGraph_pure_c
    //--------------------------------------------------------------------------

    // start the timer
    LAGraph_tic (tic) ;

    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        free (d) ;
        free (pi) ;
        LAGRAPH_OK (LAGraph_BF_pure_c (&d, &pi, s, n, nvals, I, J, W)) ;
    }

    // stop the timer
    double t3 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "BF_pure_c     time: %12.6e (sec), rate:"
        " %g (1e6 edges/sec)\n", t2, 1e-6*((double) nvals) / t3) ;
    fprintf (stderr, "speedup of BF_pure_c:      %g\n", t1/t3) ;


    //--------------------------------------------------------------------------
    // run the LAGraph_BF_full_mxv on node s
    //--------------------------------------------------------------------------

    // start the timer
    LAGraph_tic (tic) ;

    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&d2) ;
        GrB_free (&pi2) ;
        LAGRAPH_OK (LAGraph_BF_full_mxv (&d2, &pi2, &h2, AT, s)) ;
    }

    // stop the timer
    double t4 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "BF_full_mxv   time: %12.6e (sec), rate:"
        " %g (1e6 edges/sec)\n", t1, 1e-6*((double) nvals) / t1) ;
    fprintf (stderr, "speedup of BF_full_mxv:    %g\n", t1/t4) ;

    //--------------------------------------------------------------------------
    // run the BF on node s with LAGraph_BF_basic_mxv
    //--------------------------------------------------------------------------

    // start the timer
    LAGraph_tic (tic) ;

    for (int trial = 0 ; trial < ntrials ; trial++)
    {
        GrB_free (&d4) ;
        LAGRAPH_OK (LAGraph_BF_basic_mxv (&d4, AT, s)) ;
    }

    // stop the timer
    double t5 = LAGraph_toc (tic) / ntrials ;
    fprintf (stderr, "BF_basic_mxv  time: %12.6e (sec), rate:"
        " %g (1e6 edges/sec)\n", t2, 1e-6*((double) nvals) / t5) ;
    fprintf (stderr, "speedup of BF_basic_mxv:   %g\n", t1/t5) ;

    //--------------------------------------------------------------------------
    // check results
    //--------------------------------------------------------------------------
    bool isequal = false, ok = true ; 
    
    if (d != NULL && d1 != NULL)
    {
        for (int64_t i = 0 ; i < n ; i++)
        {
            double di = 0 ;
            int64_t pii = 0;
            LAGRAPH_OK (GrB_Vector_extractElement (&di, d1, i)) ;
            if (di != d[i])
            {
                fprintf (stderr, "ERROR! BF_full and BF_pure_c d  differ\n") ;
                ok = false ;
                break;
            }
            /*
            LAGRAPH_OK (GrB_Vector_extractElement (&pii, pi1, i)) ;
            if (pii != pi[i]+1)
            {
                fprintf (stderr, "ERROR! BF_full and BF_pure_c pi  differ\n") ;
                ok = false ;
                break;
            }
            */
        }
    }
    else
    {
        fprintf (stderr, "ERROR! BF_full and BF_pure_c d  differ\n") ;
        fprintf (stderr, "BF_full %s negative-weight cycle, while BF_pure_c %s "
            "negative-weight cycle\n", (d1 == NULL) ? "found":"didn't find",
            (d == NULL) ? "found":"didn't find");
        ok = false ;
    }
    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, d1, d3, NULL)) ;
    if (!isequal)
    {
        fprintf (stderr, "ERROR! BF_full and BF_basic   differ\n") ;
        ok = false ;
    }
    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, d1, d4, NULL)) ;
    if (!isequal)
    {
        fprintf (stderr, "ERROR! BF_full and BF_basic_mxv   differ\n") ;
        ok = false ;
    }
    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, d1, d2, NULL)) ;
    if (!isequal)
    {
        fprintf (stderr, "ERROR! BF_full and BF_full_mxv d   differ\n") ;
        ok = false ;
    }
/*
    LAGRAPH_OK (LAGraph_Vector_isequal (&isequal, pi1, pi2, NULL)) ;
    if (!isequal)
    {
        fprintf (stderr, "ERROR! BF_full and BF_full_mxv pi   differ\n") ;
        ok = false ;
    }
*/

    //--------------------------------------------------------------------------
    // write the result to stdout (check them outside of this main program)
    //--------------------------------------------------------------------------
    fprintf (stderr,
    "\n------------------------------------------------------------\n\n") ;
    // pure c
    if (d != NULL)
    {
        fprintf (stderr, "BF pure C result\n");
        for (int64_t i = 0 ; i < n ; i++)
        {
            fprintf (stderr, "%4.2f  ", d[i]) ;
        }
        fprintf (stderr, "\n") ;
        for (int64_t i = 0 ; i < n ; i++)
        {
            fprintf (stderr, "%4" PRIu64 "  ", pi[i]+1) ;
        }
        fprintf (stderr, "\n") ;
        fprintf (stderr, "\n") ;
    }
    // BF full
    if (d1 != NULL)
    {
        fprintf (stderr, "BF full result\n");
        for (int64_t i = 0 ; i < n ; i++)
        {
            double di = 0 ;
            LAGRAPH_OK (GrB_Vector_extractElement (&di, d1, i)) ;
            fprintf (stderr, "%4.2f  ", di) ;
        }
        fprintf (stderr, "\n") ;
        for (int64_t i = 0 ; i < n ; i++)
        {
            int64_t x = 0 ;
            LAGRAPH_OK (GrB_Vector_extractElement (&x, pi1, i)) ;
            fprintf (stderr, "%4" PRIu64 "  ", x) ;
        }
        fprintf (stderr, "\n") ;
        fprintf (stderr, "\n") ;
    }
    // BF full mxv
    if (d2 != NULL)
    {
        fprintf (stderr, "BF full mxv result\n");
        for (int64_t i = 0 ; i < n ; i++)
        {
            double di = 0 ;
            LAGRAPH_OK (GrB_Vector_extractElement (&di, d2, i)) ;
            fprintf (stderr, "%4.2f  ", di) ;
        }
        printf ("\n") ;
        for (int64_t i = 0 ; i < n ; i++)
        {
            int64_t x = 0 ;
            LAGRAPH_OK (GrB_Vector_extractElement (&x, pi2, i)) ;
            fprintf (stderr, "%4" PRIu64 "  ", x) ;
        }
        printf ("\n") ;
        printf ("\n") ;
    }
    // BF basic
    if (d3 != NULL)
    {
        fprintf (stderr, "BF basic result\n");
        for (int64_t i = 0 ; i < n ; i++)
        {
            double di = 0 ;
            LAGRAPH_OK (GrB_Vector_extractElement (&di, d3, i)) ;
            fprintf (stderr, "%4.2f  ", di) ;
        }
        fprintf (stderr, "\n") ;
        fprintf (stderr, "\n") ;
    }
    if (d4 != NULL)
    {
        fprintf (stderr, "BF basic mxv result\n");
        for (int64_t i = 0 ; i < n ; i++)
        {
            double di = 0 ;
            LAGRAPH_OK (GrB_Vector_extractElement (&di, d4, i)) ;
            fprintf (stderr, "%4.2f  ", di) ;
        }
        fprintf (stderr, "\n") ;
    }

    if (d != NULL)
    {
        fprintf (stderr, "shortest path for each node\n");
        for (int64_t i = 0 ; i < n ; i++)
        {
            int64_t node = i ;
            fprintf (stderr, "%4" PRIu64 "", node+1) ;
            while (node != s && node != -1 )
            {
                //LAGRAPH_OK (GrB_Vector_extractElement (&node, pi, node-1)) ;
                //printf (" <- %" PRIu64 "", node) ;
                node = pi[node] ;
                fprintf (stderr, " <- %4" PRIu64 "", node+1) ;
            }
            fprintf (stderr, "\n") ;
        }
    }
    //--------------------------------------------------------------------------
    // free all workspace and finish
    //--------------------------------------------------------------------------

    LAGRAPH_FREE_ALL ;
    LAGRAPH_OK (LAGraph_finalize ( )) ;
    fprintf (stderr, "BF_test: ") ;
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
