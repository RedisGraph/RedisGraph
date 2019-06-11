//------------------------------------------------------------------------------
// LAGraph/Test/LCC/lcctest.c: test program for LAGraph_lcc
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

// Usage:  lcctest < matrixmarketfile.mtx

#include "LAGraph.h"

#define LAGRAPH_FREE_ALL                            \
{                                                   \
    GrB_free (&C) ;                                 \
    GrB_free (&A) ;                                 \
    GrB_free (&M) ;                                 \
}

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------
    // initialize LAGraph and GraphBLAS
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix A = NULL, C = NULL, M = NULL ;
    GrB_Vector LCC = NULL, LCC1 = NULL ;

    LAGraph_init ( ) ;
    int nthreads_max = 1 ;
    #ifdef GxB_SUITESPARSE_GRAPHBLAS
    GxB_get (GxB_NTHREADS, &nthreads_max) ;
    #endif

    //--------------------------------------------------------------------------
    // get the input matrix
    //--------------------------------------------------------------------------

    double tic [2] ;
    LAGraph_tic (tic) ;

    FILE *f ;
    if (argc == 1)
    {
        f = stdin ;
    }
    else
    {
        f = fopen (argv[0], "r") ;
        if (f == NULL)
        {
            printf ("unable to open file [%s]\n", argv[0]) ;
            return (GrB_INVALID_VALUE) ;
        }
    }
    LAGRAPH_OK (LAGraph_mmread (&C, f)) ;
    double t_read = LAGraph_toc (tic) ;
    fprintf (stderr, "\nread A time:     %14.6f sec\n", t_read) ;

    LAGraph_tic (tic) ;
    GrB_Index n, ne ;
    LAGRAPH_OK (GrB_Matrix_nrows (&n, C)) ;

#if 0
    // A = spones (C), and typecast to FP64
    LAGRAPH_OK (GrB_Matrix_new (&A, GrB_FP64, n, n)) ;
    LAGRAPH_OK (GrB_apply (A, NULL, NULL, LAGraph_ONE_FP64, C, NULL)) ;
    GrB_free (&C) ;

    // M = diagonal mask matrix
    LAGRAPH_OK (GrB_Matrix_new (&M, GrB_BOOL, n, n)) ;
    for (int64_t i = 0 ; i < n ; i++)
    {
        // M(i,i) = true ;
        LAGRAPH_OK (GrB_Matrix_setElement (M, (bool) true, i, i)) ;
    }

    // remove self edges (via M)
    LAGRAPH_OK (GrB_assign (A, M, NULL, A, GrB_ALL, n, GrB_ALL, n,
        LAGraph_desc_oocr)) ;
    GrB_free (&M) ;

    LAGRAPH_OK (GrB_Matrix_nvals (&ne, A)) ;
#else
    A = C ;
    C = NULL ;
#endif

    double t_process = LAGraph_toc (tic) ;
    fprintf (stderr, "process A time:  %14.6f sec\n", t_process) ;

    //--------------------------------------------------------------------------
    // compute LCC
    //--------------------------------------------------------------------------

    double t1 ;
    for (int nthreads = 1 ; nthreads <= nthreads_max ; )
    {
        #ifdef GxB_SUITESPARSE_GRAPHBLAS
        GxB_set (GxB_NTHREADS, nthreads) ;
        #endif

        double tic [2] ;
        LAGraph_tic (tic) ;
        LAGRAPH_OK (LAGraph_lcc (&LCC, A, true)) ;
        double t = LAGraph_toc (tic) ;

        if (nthreads == 1)
        {
            LCC1 = LCC ;
            LCC = NULL ;
            t1 = t ;
            // GxB_print (LCC, 3) ;
            // dump the result to stdout
            for (GrB_Index i = 0 ; i < n ; i++)
            {
                double x = 0 ;
                LAGRAPH_OK (GrB_Vector_extractElement (&x, LCC1, i)) ;
                if (info == GrB_NO_VALUE) printf (" 0.\n") ;
                else printf ("%32.16g\n", x) ;
            }
        }
        else
        {
            bool ok ;
            LAGRAPH_OK (LAGraph_Vector_isequal (&ok, LCC, LCC1, GrB_EQ_FP64)) ;
            if (!ok) { fprintf (stderr, "error!\n") ; abort ( ) ; }
            GrB_free (&LCC) ;
        }

        fprintf (stderr, "nthreads: %3d time: %12.6f rate: %6.2f", nthreads, t,
            1e-6 * ne / t) ;
        if (nthreads > 1)
        {
            fprintf (stderr, " speedup: %6.2f", t1 / t) ;
        }
        fprintf (stderr, "\n") ;

        if (nthreads != nthreads_max && 4 * nthreads > nthreads_max)
        {
            nthreads = nthreads_max ;
        }
        else
        {
            nthreads *= 4 ;
        }
    }

    fprintf (stderr, "\n") ;
    LAGRAPH_FREE_ALL ;
    LAGraph_finalize ( ) ;
}

