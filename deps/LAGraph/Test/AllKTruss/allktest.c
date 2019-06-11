//------------------------------------------------------------------------------
// LAGraph/Test/AllKTruss/allktest.c: test program for LAGraph_ktruss
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

// Usage:  allktest < matrixmarketfile.mtx
// Contributed by Tim Davis, Texas A&M

#include "LAGraph.h"

#define LAGRAPH_FREE_ALL                            \
{                                                   \
    if (Cset != NULL)                               \
    {                                               \
        for (int64_t kk = 3 ; kk <= kmax ; kk++)    \
        {                                           \
            GrB_free (&(Cset [kk])) ;               \
        }                                           \
    }                                               \
    GrB_free (&A) ;                                 \
    GrB_free (&M) ;                                 \
    LAGRAPH_FREE (ntris) ;                          \
    LAGRAPH_FREE (nedges) ;                         \
    LAGRAPH_FREE (nstepss) ;                        \
}

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------
    // initialize LAGraph and GraphBLAS
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix A = NULL, C = NULL, M = NULL ;
    GrB_Matrix *Cset = NULL ;
    int64_t kmax = 0 ; 
    int64_t *ntris = NULL, *nedges  = NULL, *nstepss = NULL ;

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
    GrB_Index n, ne ;
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

    LAGRAPH_OK (GrB_Matrix_nvals (&ne, A)) ;

    double t_process = LAGraph_toc (tic) ;
    printf ("process A time:  %14.6f sec\n", t_process) ;

    //--------------------------------------------------------------------------
    // construct all ktrusses
    //--------------------------------------------------------------------------

    Cset = NULL ; // LAGraph_malloc (n, sizeof (GrB_Matrix)) ;
    ntris   = LAGraph_malloc (n, sizeof (int64_t)) ;
    nedges  = LAGraph_malloc (n, sizeof (int64_t)) ;
    nstepss = LAGraph_malloc (n, sizeof (int64_t)) ;

    if (ntris == NULL || nedges == NULL || nstepss == NULL)
    {
        printf ("out of memory\n") ;
        LAGRAPH_FREE_ALL ;
    }

    double t1 ;
    for (int nthreads = 1 ; nthreads <= nthreads_max ; )
    {
        #ifdef GxB_SUITESPARSE_GRAPHBLAS
        GxB_set (GxB_NTHREADS, nthreads) ;
        #endif

        double tic [2] ;
        LAGraph_tic (tic) ;
        LAGRAPH_OK (LAGraph_allktruss (Cset, A, &kmax, ntris, nedges, nstepss));
        double t = LAGraph_toc (tic) ;

        if (nthreads == 1)
        {
            t1 = t ;
            for (int64_t kk = 3 ; kk <= kmax ; kk++)
            {
                printf (" k %4d", kk) ;
                printf (" edges %12" PRIu64, nedges [kk]) ;
                printf (" ntriangles %12" PRId64, ntris [kk]) ;
                printf (" nsteps %6" PRId64 "\n", nstepss [kk]) ;
            }
        }

        printf ("nthreads: %3d time: %12.6f rate: %6.2f", nthreads, t,
            1e-6 * ne / t) ;
        if (nthreads > 1)
        {
            printf (" speedup: %6.2f", t1 / t) ;
        }
        printf ("\n") ;

        if (nthreads != nthreads_max && 4 * nthreads > nthreads_max)
        {
            nthreads = nthreads_max ;
        }
        else
        {
            nthreads *= 4 ;
        }
    }

    printf ("\n") ;
    LAGRAPH_FREE_ALL ;
    LAGraph_finalize ( ) ;
}

