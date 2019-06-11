//------------------------------------------------------------------------------
// LAGraph/Test/TriangeCount/ttest.c: test program for LAGraph_tricount
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

// Usage:  ttest < matrixmarketfile.mtx

// Three methods are tested: Sandia, Sandia2, and SandiaDot, with nthreads of
// 1, 2, 4, ..., max # of threads.  The prep time (to compute L and/or U) is
// not included in the total time.

#include "LAGraph.h"

#define LAGRAPH_FREE_ALL    \
{                           \
    GrB_free (&Support) ;   \
    GrB_free (&C) ;         \
    GrB_free (&M) ;         \
    GrB_free (&A) ;         \
    GrB_free (&E) ;         \
    GrB_free (&L) ;         \
    GrB_free (&U) ;         \
    LAGRAPH_FREE (I) ;      \
    LAGRAPH_FREE (J) ;      \
    LAGRAPH_FREE (X) ;      \
}

void print_method (int method)
{
    switch (method)
    {
        case 0:  printf ("minitri:    nnz (A*E == 2) / 3\n"  ) ; break ;
        case 1:  printf ("Burkhardt:  sum ((A^2) .* A) / 6\n") ; break ;
        case 2:  printf ("Cohen:      sum ((L*U) .* A) / 2\n") ; break ;
        case 3:  printf ("Sandia:     sum ((L*L) .* L)\n"    ) ; break ;
        case 4:  printf ("Sandia2:    sum ((U*U) .* U)\n"    ) ; break ;
        case 5:  printf ("SandiaDot:  sum ((L*U') .* L)\n"   ) ; break ;
        case 6:  printf ("SandiaDot2: sum ((U*L') .* U)\n"   ) ; break ;
        default: printf ("invalid method\n") ; abort ( ) ;
    }
}

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------
    // initialize LAGraph and GraphBLAS
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix A = NULL, E = NULL, L = NULL, U = NULL, C = NULL, M = NULL ;
    GrB_Vector Support = NULL ;
    GrB_Index *I = NULL, *J = NULL ;
    uint32_t *X = NULL ;
    LAGraph_init ( ) ;

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

    double t_process = LAGraph_toc (tic) ;
    printf ("process A time:  %14.6f sec\n", t_process) ;

//  GxB_print (A, 3) ;

    //--------------------------------------------------------------------------
    // construct L and U
    //--------------------------------------------------------------------------

    #ifdef GxB_SUITESPARSE_GRAPHBLAS

        // U = triu (A,1), for methods 2, 3, and 5
        LAGraph_tic (tic) ;
        int64_t k = 1 ;
        LAGRAPH_OK (GrB_Matrix_new (&U, GrB_UINT32, n, n)) ;
        LAGRAPH_OK (GrB_Vector_new (&Support, GrB_INT64, 1)) ;
        LAGRAPH_OK (GrB_Vector_setElement (Support, k, 0)) ;
        LAGRAPH_OK (GxB_select (U, NULL, NULL, GxB_TRIU, A,
                #if GxB_IMPLEMENTATION >= GxB_VERSION (3,0,0)
                Support,    // V3.0.0 and later uses a GrB_Vector
                #else
                &k,         // V2.x and earlier uses a (const void *) pointer
                #endif
                NULL)) ;

        LAGRAPH_OK (GrB_Matrix_nvals (&nedges, U)) ;
        printf ("n %.16g # edges %.16g\n", (double) n, (double) nedges) ;
        double t_U = LAGraph_toc (tic) ;
        printf ("U=triu(A) time:  %14.6f sec\n", t_U) ;

        // L = tril (A,-1), for methods 2 and 4
        LAGraph_tic (tic) ;
        LAGRAPH_OK (GrB_Matrix_new (&L, GrB_UINT32, n, n)) ;
        k = -1 ;
        LAGRAPH_OK (GrB_Vector_setElement (Support, k, 0)) ;
        LAGRAPH_OK (GxB_select (L, NULL, NULL, GxB_TRIL, A,
                #if GxB_IMPLEMENTATION >= GxB_VERSION (3,0,0)
                Support,    // V3.0.0 and later uses a GrB_Vector
                #else
                &k,         // V2.x and earlier uses a (const void *) pointer
                #endif
                NULL)) ;

        double t_L = LAGraph_toc (tic) ;
        printf ("L=tril(A) time:  %14.6f sec\n", t_L) ;

    #else

        // build both L and U using extractTuples (slower than GxB_select)
        LAGraph_tic (tic) ;
        GrB_Index nvals ;
        LAGRAPH_OK (GrB_Matrix_nvals (&nvals, A)) ;
        I = LAGraph_malloc (nvals, sizeof (GrB_Index)) ;
        J = LAGraph_malloc (nvals, sizeof (GrB_Index)) ;
        X = LAGraph_malloc (nvals, sizeof (uint32_t)) ;
        if (I == NULL || J == NULL || X == NULL)
        {
            LAGRAPH_FREE_ALL ;
            printf ("out of memory\n") ;
            abort ( ) ;
        }
        LAGRAPH_OK (GrB_Matrix_extractTuples (I, J, X, &nvals, A)) ;

        // remove entries in the upper triangular part
        nedges = 0 ;
        for (int64_t k = 0 ; k < nvals ; k++)
        {
            if (I [k] > J [k])
            {
                // keep this entry
                I [nedges] = I [k] ;
                J [nedges] = J [k] ;
                X [nedges] = X [k] ;
                nedges++ ;
            }
        }

        LAGRAPH_OK (GrB_Matrix_new (&L, GrB_UINT32, n, n)) ;
        LAGRAPH_OK (GrB_Matrix_new (&U, GrB_UINT32, n, n)) ;

        LAGRAPH_OK (GrB_Matrix_build (L, I, J, X, nedges, GrB_PLUS_UINT32)) ;
        LAGRAPH_OK (GrB_Matrix_build (U, J, I, X, nedges, GrB_PLUS_UINT32)) ;

        LAGRAPH_FREE (I) ;
        LAGRAPH_FREE (J) ;
        LAGRAPH_FREE (X) ;

        double t_LU = LAGraph_toc (tic) ;
        printf ("L and U   time:  %14.6f sec\n", t_LU) ;

    #endif

    // E is not constructed (required for method 0, which is very slow)

    //--------------------------------------------------------------------------
    // triangle counting
    //--------------------------------------------------------------------------

    int nthreads_max = 1 ;
    #if defined ( GxB_SUITESPARSE_GRAPHBLAS )
    GxB_get (GxB_NTHREADS, &nthreads_max) ;
    #endif

    // warmup for more accurate timing
    int64_t ntriangles ;
    double t [2] ;
    LAGRAPH_OK (LAGraph_tricount (&ntriangles, 3, A, E, L, U, t)) ;
    printf ("# of triangles: %" PRId64 "\n", ntriangles) ;

    double t_best = INFINITY ;
    int method_best = -1 ;
    int nthreads_best = -1 ;

    // try methods 3 to 6
    for (int method = 3 ; method <= 6 ; method++)
    {

        printf ("\nMethod: ") ;
        print_method (method) ;

        double t_sequential ;
        for (int nthreads = 1 ; nthreads <= nthreads_max ; )
        {
            #ifdef GxB_SUITESPARSE_GRAPHBLAS
            GxB_set (GxB_NTHREADS, nthreads) ;
            #endif
            int64_t nt ;
            LAGRAPH_OK (LAGraph_tricount (&nt, method, A, E, L, U, t)) ;
            double ttot = t [0] + t [1] ;
            printf ("nthreads: %3d time: %12.6f rate: %6.2f", nthreads, ttot,
                1e-6 * nedges / ttot) ;
            if (nthreads == 1)
            {
                t_sequential = ttot ;
            }
            else
            {
                printf (" speedup: %6.2f", t_sequential / ttot) ;
            }
            printf ("\n") ;
            if (nt != ntriangles) { printf ("Test failure!\n") ; abort ( ) ; }

            if (ttot < t_best)
            {
                t_best = ttot ;
                method_best = method ;
                nthreads_best = nthreads ;
            }

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

    printf ("\nBest method:\n") ;
    printf ("nthreads: %3d time: %12.6f rate: %6.2f ", nthreads_best, t_best,
        1e-6 * nedges / t_best) ;
    print_method (method_best) ;
    LAGRAPH_FREE_ALL ;
    LAGraph_finalize ( ) ;

    printf ("\n") ;
}

