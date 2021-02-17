//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/pagerank_demo.c: PageRank via various semirings
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Read a graph from a file and compute the pagerank of its nodes, using
// dpagerank and ipagerank.

// usage:  ./pagerank_demo   < file
//         ./pagerank_demo 0 < file
//         ./pagerank_demo 1 < file
//
// default is 0-based, for the matrices in the Matrix/ folder
//
// The infile has one line per edge in the graph; these have the form
//
//  i j x
//
// where A(i,j)=x is performed by GrB_Matrix_build, to construct the matrix.
// The dimensions of A are assumed to be the largest row and column indices,
// plus one (in read_matrix.c).
//
// A must be square.

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL                    \
{                                   \
    GrB_Matrix_free (&A) ;          \
    if (Pd != NULL) free (Pd) ;     \
    if (Pi != NULL) free (Pi) ;     \
    if (P2 != NULL) free (P2) ;     \
}

#include "graphblas_demos.h"

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix A = NULL ;
    PageRank *Pd = NULL, *P2 = NULL ;
    iPageRank *Pi = NULL ;

    double tic [2], t ;
    OK (GrB_init (GrB_NONBLOCKING)) ;
    int nthreads ;
    OK (GxB_Global_Option_get (GxB_GLOBAL_NTHREADS, &nthreads)) ;
    fprintf (stderr, "\npagerank_demo: nthreads: %d\n", nthreads) ;
    printf  (        "\npagerank_demo: nthreads: %d\n", nthreads) ;

    //--------------------------------------------------------------------------
    // read a matrix from stdin
    //--------------------------------------------------------------------------

    bool one_based = false ;
    if (argc > 1) one_based = strtol (argv [1], NULL, 0) ;

    OK (read_matrix (&A,
        stdin,      // read matrix from stdin
        false,      // unsymmetric
        false,      // self edges OK
        one_based,  // 0-based or 1-based, depending on input arg
        true,       // read input file as Boolean
        true)) ;    // print status to stdout

    GrB_Index n, nvals ;
    OK (GrB_Matrix_nrows (&n, A)) ;
    OK (GrB_Matrix_nvals (&nvals, A)) ;

    //--------------------------------------------------------------------------
    // compute the page rank via a real semiring
    //--------------------------------------------------------------------------

    simple_tic (tic) ;
    OK (dpagerank (&Pd, A)) ;
    t = simple_toc (tic) ;

    fprintf (stderr, "n %g edges %g  dpagerank time : %14.6f iters: 20\n",
        (double) n, (double) nvals, t) ;
    printf  (        "n %g edges %g  dpagerank time : %14.6f iters: 20\n",
        (double) n, (double) nvals, t) ;

    //--------------------------------------------------------------------------
    // compute the page rank via an integer semiring
    //--------------------------------------------------------------------------

    simple_tic (tic) ;
    OK (ipagerank (&Pi, A)) ;
    t = simple_toc (tic) ;

    fprintf (stderr, "n %g edges %g  ipagerank time : %14.6f iters: 20\n",
        (double) n, (double) nvals, t) ;
    printf  (        "n %g edges %g  ipagerank time : %14.6f iters: 20\n",
        (double) n, (double) nvals, t) ;

    //--------------------------------------------------------------------------
    // compute the page rank via an extreme semiring
    //--------------------------------------------------------------------------

    int iters ;
    simple_tic (tic) ;
    OK (dpagerank2 (&P2, A, 100, 1e-5, &iters, GxB_DEFAULT)) ;
    t = simple_toc (tic) ;

    fprintf (stderr, "n %g edges %g  dpagerank time : %14.6f iters: %d\n",
        (double) n, (double) nvals, t, iters) ;
    printf  (        "n %g edges %g  dpagerank time : %14.6f iters: %d\n",
        (double) n, (double) nvals, t, iters) ;

    //--------------------------------------------------------------------------
    // print results
    //--------------------------------------------------------------------------

    int64_t limit = MIN (n, 5000) ;
    printf ("Top %g nodes:\n", (double) limit) ;
    for (int64_t i = 0 ; i < limit ; i++)
    {
        printf ("%5g d:[%6g : %16.8e] i:[%6g : %16.8e] x:[%6g : %16.8e]",
            (double) i,
            (double) Pd [i].page, (double) Pd [i].pagerank,
            (double) Pi [i].page, (double) Pi [i].pagerank,
            (double) P2 [i].page, (double) P2 [i].pagerank) ;
        if (Pd [i].page != Pi [i].page || Pd [i].page != P2 [i].page)
        {
            printf ("mismatch") ;
        }
        printf ("\n") ;
    }

    //--------------------------------------------------------------------------
    // free all workspace
    //--------------------------------------------------------------------------

    FREE_ALL ;
    GrB_finalize ( ) ;
}

