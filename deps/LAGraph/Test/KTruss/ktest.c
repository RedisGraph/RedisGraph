//------------------------------------------------------------------------------
// LAGraph/Test/KTruss/ktest.c: test program for LAGraph_ktruss
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

// Usage:  ktest < matrixmarketfile.mtx

#include "LAGraph.h"

#define LAGRAPH_FREE_ALL    \
{                           \
    GrB_free (&C) ;         \
    GrB_free (&A) ;         \
    GrB_free (&M) ;         \
}

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------
    // initialize LAGraph and GraphBLAS
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix A = NULL, C = NULL, M = NULL ;
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
    printf ("\nread A time:     %14.6f sec\n", t_read) ;

    LAGraph_tic (tic) ;
    GrB_Index n, nedges ;
    LAGRAPH_OK (GrB_Matrix_nrows (&n, C)) ;

    // A = spones (C), and typecast to uint32
    LAGRAPH_OK (GrB_Matrix_new (&A, GrB_UINT32, n, n)) ;
    LAGRAPH_OK (GrB_apply (A, NULL, NULL, LAGraph_ONE_UINT32, C, NULL)) ;
    GrB_free (&C) ;

    // M = diagonal mask matrix
    LAGRAPH_OK (GrB_Matrix_new (&M, GrB_BOOL, n, n)) ;
    for (int64_t i = 0 ; i < n ; i++)
    {
        // M(i,i) = true ;
        LAGRAPH_OK (GrB_Matrix_setElement (M, (bool) true, i, i)) ;
    }

    // make A symmetric (A = spones (A+A')) and remove self edges (via M)
    LAGRAPH_OK (GrB_eWiseAdd (A, M, NULL, LAGraph_LOR_UINT32, A, A,
        LAGraph_desc_otcr)) ;
    GrB_free (&M) ;

    LAGRAPH_OK (GrB_Matrix_nvals (&nedges, A)) ;

    double t_process = LAGraph_toc (tic) ;
    printf ("process A time:  %14.6f sec\n", t_process) ;

    //--------------------------------------------------------------------------
    // construct all ktrusses
    //--------------------------------------------------------------------------

    int64_t nt ;
    GrB_Index nedges_in_ktruss = 1 ;

    for (uint32_t k = 3 ; k < 10 && nedges_in_ktruss > 0 ; k++)
    {

        printf ("\nKTruss: k = %3d:", (int) k) ;
        double t1 ;

        double t_sequential ;
        for (int nthreads = 1 ; nthreads <= nthreads_max ; )
        {
            #ifdef GxB_SUITESPARSE_GRAPHBLAS
            GxB_set (GxB_NTHREADS, nthreads) ;
            #endif

            double tic [2] ;
            LAGraph_tic (tic) ;

            int32_t nsteps ;
            LAGRAPH_OK (LAGraph_ktruss (&C, A, k, &nsteps)) ;
            LAGRAPH_OK (GrB_Matrix_nvals (&nedges_in_ktruss, C)) ;
            double t = LAGraph_toc (tic) ;

            if (nthreads == 1)
            {
                t1 = t ;
                LAGRAPH_OK (GrB_reduce (&nt, NULL, GxB_PLUS_INT64_MONOID,   
                    C, NULL)) ;
                nt /= 6 ;
                printf (" edges %" PRIu64, nedges_in_ktruss/2) ;
                printf (" ntriangles %" PRId64 "\n", nt) ;
            }

            GrB_free (&C) ;

            printf ("nthreads: %3d time: %12.6f rate: %6.2f", nthreads, t,
                1e-6 * nedges / t) ;
            if (nthreads > 1)
            {
                printf (" speedup: %6.2f", t1 / t) ;
            }
            printf ("\n") ;

            if (nthreads != nthreads_max && 2 * nthreads > nthreads_max)
            {
                nthreads = nthreads_max ;
            }
            else
            {
                nthreads *= 2 ;
            }
        }
    }

    printf ("\n") ;
    LAGRAPH_FREE_ALL ;
    LAGraph_finalize ( ) ;
}

