//------------------------------------------------------------------------------
// LAGraph_pagerank: pagerank using a real semiring
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

// LAGraph_pagerank: pagerank using a real semiring, contributed by Tim Davis,
// Texas A&M.

// A is a square unsymmetric binary matrix of size n-by-n, where A(i,j) is the
// edge (i,j).  Self-edges are LAGRAPH_OK.  A can be of any built-in type.

// On output, P is pointer to an array of LAGraph_PageRank structs.  P[0] is the
// highest ranked page, with pagerank P[0].pagerank and the page corresponds to
// node number P[0].page in the graph.  P[1] is the next page, and so on, to
// the lowest-ranked page P[n-1].page with rank P[n-1].pagerank.

// See LAGraph/Test/PageRank/dpagerank.m for the equivalent computation in
// MATLAB (except the random number generator differs).

//------------------------------------------------------------------------------
// helper macros
//------------------------------------------------------------------------------

// free all workspace
#define FREEWORK                                \
{                                               \
    GrB_free (&T) ;                             \
    GrB_free (&D) ;                             \
    GrB_free (&C) ;                             \
    GrB_free (&r) ;                             \
    GrB_free (&t) ;                             \
    GrB_free (&d) ;                             \
    LAGRAPH_FREE (I) ;                          \
    LAGRAPH_FREE (X) ;                          \
    GrB_free (&op_div) ;                        \
    GrB_free (&op_teleport) ;                   \
    GrB_free (&op_diff) ;                       \
}

// error handler: free output P and all workspace
#define LAGRAPH_FREE_ALL                        \
{                                               \
    LAGRAPH_FREE (P) ;                          \
    FREEWORK ;                                  \
}

#include "LAGraph.h"

//------------------------------------------------------------------------------
// scalar operators
//------------------------------------------------------------------------------

#define DAMPING 0.85

// TODO uses global values.  Avoid them!  LAGraph_pagerank can use internal
// parallelism (all threads will see the same global values), but multiple
// instances of LAGraph_pagerank called be done in parallel in a user
// application.  Replace globals via binary operator on expanded scalars, or
// GrB_mxm.
float rsum ;
float teleport_scalar ;

void fdiv  (void *z, const void *x)
{
    (*((float *) z)) = (* ((float *) x)) / rsum ;
}

void fteleport  (void *z, const void *x)
{
    (*((float *) z)) = (* ((float *) x)) + teleport_scalar ;
}

void fdiff (void *z, const void *x, const void *y)
{
    float delta = (* ((float *) x)) - (* ((float *) y)) ;
    (*((float *) z)) = delta * delta ;
}

//------------------------------------------------------------------------------
// comparison function for qsort
//------------------------------------------------------------------------------

int compar (const void *x, const void *y)
{
    LAGraph_PageRank *a = (LAGraph_PageRank *) x ;
    LAGraph_PageRank *b = (LAGraph_PageRank *) y ;

    // sort by pagerank in descending order
    if (a->pagerank > b->pagerank)
    {
        return (-1) ;
    }
    else if (a->pagerank == b->pagerank)
    {
        return (0) ;
    }
    else
    {
        return (1) ;
    }
}

//------------------------------------------------------------------------------
// LAGraph_pagerank: compute the pagerank of all nodes in a graph
//------------------------------------------------------------------------------

GrB_Info LAGraph_pagerank       // GrB_SUCCESS or error condition
(
    LAGraph_PageRank **Phandle, // output: array of LAGraph_PageRank structs
    GrB_Matrix A,               // binary input graph, not modified
    int itermax,                // max number of iterations
    double tol,                 // stop when norm (r-rnew,2) < tol
    int *iters                  // number of iterations taken
)
{

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    double tic [2] ;
    LAGraph_tic (tic) ;

    GrB_Info info ;
    float *X = NULL ;
    GrB_Index n, nvals, *I = NULL ;
    LAGraph_PageRank *P = NULL ;
    GrB_Vector r = NULL, t = NULL, d = NULL ;
    GrB_UnaryOp op_div = NULL ;
    GrB_UnaryOp op_teleport = NULL ;
    GrB_BinaryOp op_diff = NULL ;
    GrB_Matrix C = NULL, D = NULL, T = NULL ;

    if (Phandle == NULL) LAGRAPH_ERROR ("null pointer", GrB_NULL_POINTER) ;
    (*Phandle) = NULL ;

    // n = size (A,1) ;         // number of nodes
    LAGRAPH_OK (GrB_Matrix_nrows (&n, A)) ;
    if (n == 0) return (GrB_SUCCESS) ;

    // teleport = (1 - 0.85) / n
    float one = 1.0 ;
    float teleport = (one - DAMPING) / ((float) n) ;

    // r (i) = 1/n for all nodes i
    float x = 1.0 / ((float) n) ;
    LAGRAPH_OK (GrB_Vector_new (&r, GrB_FP32, n)) ;
    LAGRAPH_OK (GrB_assign (r, NULL, NULL, x, GrB_ALL, n, NULL)) ;

    // d (i) = out deg of node i
    LAGRAPH_OK (GrB_Vector_new (&d, GrB_FP32, n)) ;
    LAGRAPH_OK (GrB_reduce (d, NULL, NULL, GrB_PLUS_FP32, A, NULL)) ;

    // D = (1/diag (d)) * DAMPING
    #ifdef GxB_SUITESPARSE_GRAPHBLAS
    GrB_Type type ;
    LAGRAPH_OK (GxB_Vector_export (&d, &type, &n, &nvals, &I, (void **) (&X),
        NULL)) ;
    #else
    nvals = n ;
    I = LAGraph_malloc ((nvals + 1), sizeof (GrB_Index)) ;
    X = LAGraph_malloc ((nvals + 1), sizeof (float)) ;
    if (I == NULL || X == NULL)
    {
        LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
    }
    LAGRAPH_OK (GrB_Vector_extract (I, X, &nvals, d)) ;
    LAGRAPH_OK (GrB_free (&d)) ;
    #endif

    for (int64_t k = 0 ; k < nvals ; k++) X [k] = DAMPING / X [k] ;
    LAGRAPH_OK (GrB_Matrix_new (&D, GrB_FP32, n, n)) ;
    LAGRAPH_OK (GrB_Matrix_build (D, I, I, X, nvals, GrB_PLUS_FP32)) ;
    LAGRAPH_FREE (I) ;
    LAGRAPH_FREE (X) ;

    // GxB_print (D, 3) ;

    // C = diagonal matrix with all zeros on the diagonal.  This ensures that
    // the vectors r and t remain dense, which is faster, and is required
    // for the t += teleport_scalar step.
    LAGRAPH_OK (GrB_Matrix_new (&C, GrB_FP32, n, n)) ;
    // GxB_set (C, GxB_HYPER, GxB_ALWAYS_HYPER) ;

    for (int64_t k = 0 ; k < n ; k++)
    {
        // C(k,k) = 0
        LAGRAPH_OK (GrB_Matrix_setElement (C, (float) 0, k, k)) ;
    }

    // make sure D is diagonal
    LAGRAPH_OK (GrB_eWiseAdd (D, NULL, NULL, GrB_PLUS_FP32, D, C, NULL)) ;

    // GxB_print (A, 3) ;

    // GxB_print (C, 3) ;

//  int nthreads_save ;
//  GxB_get (GxB_NTHREADS, &nthreads_save) ;
//  GxB_set (GxB_NTHREADS, 1);

    double tt = LAGraph_toc (tic) ;
    fprintf (stderr, "\ninit time %g\n", tt) ;
    LAGraph_tic (tic) ;

    // use GrB_mxv for t=C*r below
    // C = C+(D*A)' = C+A'*D'  : using the transpose of C, and C*r below
//  LAGRAPH_OK (GrB_mxm (C, NULL, GrB_PLUS_FP32, GxB_PLUS_TIMES_FP32, A, D,
//      LAGraph_desc_ttoo)) ;

    // T = D*A
    LAGRAPH_OK (GrB_Matrix_new (&T, GrB_FP32, n, n)) ;
    LAGRAPH_OK (GrB_mxm (T, NULL, NULL, GxB_PLUS_TIMES_FP32, D, A, NULL)) ;

    tt = LAGraph_toc (tic) ;
    fprintf (stderr, "D*A mxm time %g\n", tt) ;
    LAGraph_tic (tic) ;

    // C = C+T'
    LAGRAPH_OK (GrB_transpose (C, NULL, GrB_PLUS_FP32, T, NULL)) ;

    LAGRAPH_OK (GrB_free (&T)) ;

    LAGRAPH_OK (GrB_free (&D)) ;

    // create operators
    LAGRAPH_OK (GrB_UnaryOp_new (&op_div,  fdiv, GrB_FP32, GrB_FP32)) ;
    LAGRAPH_OK (GrB_UnaryOp_new (&op_teleport, fteleport, GrB_FP32, GrB_FP32)) ;
    LAGRAPH_OK (GrB_BinaryOp_new (&op_diff, fdiff,
        GrB_FP32, GrB_FP32, GrB_FP32)) ;

    float ftol = tol*tol ;  // use tol^2 so sqrt(rdiff) not needed
    float rdiff = 1 ;       // so first iteration is always done

    LAGRAPH_OK (GrB_Vector_new (&t, GrB_FP32, n)) ;

    // GxB_print (C, 2) ;
    // GxB_fprint (C, 2, stderr) ;

    tt = LAGraph_toc (tic) ;
    fprintf (stderr, "C transpose time %g\n", tt) ;
    LAGraph_tic (tic) ;
//  GxB_set (GxB_NTHREADS, nthreads_save) ;

    //--------------------------------------------------------------------------
    // iterate to compute the pagerank of each node
    //--------------------------------------------------------------------------

    double t2 = 0 ;
    double t5 = 0 ;
    double t6 = 0 ;
    double t7 = 0 ;
    double t8 = 0 ;

    for ((*iters) = 0 ; (*iters) < itermax && rdiff > ftol ; (*iters)++)
    {

        //----------------------------------------------------------------------
        // t = (r*C or C*r) + (teleport * sum (r)) ;
        //----------------------------------------------------------------------
        double tic2 [2] ;

        // GxB_print (r, 2) ;

        LAGraph_tic (tic2) ;
        LAGRAPH_OK (GrB_reduce (&rsum, NULL, GxB_PLUS_FP32_MONOID, r, NULL)) ;
        t5 += LAGraph_toc (tic2) ;

        LAGraph_tic (tic2) ;
        // t = C*r
        // using the transpose of A, scaled (dot product)
        LAGRAPH_OK (GrB_mxv (t, NULL, NULL, GxB_PLUS_TIMES_FP32, C, r, NULL)) ;

        t2 += LAGraph_toc (tic2) ;
//      fprintf (stderr, "one mxv %g\n", t3) ;

//      LAGraph_tic (tic2) ;
//      LAGRAPH_OK (GrB_mxv (t, NULL, NULL, GxB_PLUS_TIMES_FP32, C, r, NULL)) ;
//      t3 = LAGraph_toc (tic2) ;
//      fprintf (stderr, "another mxv %g\n", t3) ;

        // t += teleport_scalar ;
        LAGraph_tic (tic2) ;
        teleport_scalar = teleport * rsum ;
//      LAGRAPH_OK (GrB_assign (t, NULL, GrB_PLUS_FP32, teleport_scalar,
//          GrB_ALL, n, NULL)) ;
        LAGRAPH_OK (GrB_apply (t, NULL, NULL, op_teleport, t, NULL)) ;
        t6 += LAGraph_toc (tic2) ;

        //----------------------------------------------------------------------
        // rdiff = sum ((r-t).^2)
        //----------------------------------------------------------------------

        LAGraph_tic (tic2) ;
        LAGRAPH_OK (GrB_eWiseAdd (r, NULL, NULL, op_diff, r, t, NULL)) ;
        t7 += LAGraph_toc (tic2) ;

        LAGraph_tic (tic2) ;
        LAGRAPH_OK (GrB_reduce (&rdiff, NULL, GxB_PLUS_FP32_MONOID, r, NULL)) ;
        t8 += LAGraph_toc (tic2) ;

        //----------------------------------------------------------------------
        // swap r and t
        //----------------------------------------------------------------------

        GrB_Vector temp = r ;
        r = t ;
        t = temp ;
//      LAGRAPH_OK (GrB_Vector_clear (t)) ;

        // GxB_print (r, 3) ;
    }

        fprintf (stderr, "reduce1  %g\n", t5) ;
        fprintf (stderr, "mxv      %g\n", t2) ;
        fprintf (stderr, "teleport %g\n", t6) ;
        fprintf (stderr, "add      %g\n", t7) ;
        fprintf (stderr, "reduce2  %g\n", t8) ;

    LAGRAPH_OK (GrB_free (&C)) ;
    LAGRAPH_OK (GrB_free (&t)) ;

    tt = LAGraph_toc (tic) ;
    fprintf (stderr, "rank time %g\n", tt) ;
    LAGraph_tic (tic) ;

    //--------------------------------------------------------------------------
    // scale the result
    //--------------------------------------------------------------------------

    // rsum = sum (r)
    LAGRAPH_OK (GrB_reduce (&rsum, NULL, GxB_PLUS_FP32_MONOID, r, NULL)) ;

    // TODO use vxm with rsum as a 1-by-1 matrix
    // r = r / rsum
    LAGRAPH_OK (GrB_apply (r, NULL, NULL, op_div, r, NULL)) ;

    tt = LAGraph_toc (tic) ;
    fprintf (stderr, "scale time %g\n", tt) ;
    LAGraph_tic (tic) ;

    //--------------------------------------------------------------------------
    // sort the nodes by pagerank
    //--------------------------------------------------------------------------

    // [r,irank] = sort (r, 'descend') ;

    // extract the contents of r
    #ifdef GxB_SUITESPARSE_GRAPHBLAS
    LAGRAPH_OK (GxB_Vector_export (&r, &type, &n, &nvals, &I, (void **) (&X),
        NULL)) ;
    #else
    nvals = n ;
    I = LAGraph_malloc (n, sizeof (GrB_Index)) ;
    X = LAGraph_malloc (n, sizeof (float)) ;
    if (I == NULL || X == NULL)
    {
        LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
    }
    LAGRAPH_OK (GrB_Vector_extractTuples (I, X, &nvals, r)) ;
    LAGRAPH_OK (GrB_free (&r)) ;
    #endif

    // this will always be true since r is dense, but check anyway:
    if (nvals != n) return (GrB_PANIC) ;

    // P = struct (X,I)
    P = LAGraph_malloc (n, sizeof (LAGraph_PageRank)) ;
    if (P == NULL)
    {
        LAGRAPH_ERROR ("out of memory", GrB_OUT_OF_MEMORY) ;
    }
    for (int64_t k = 0 ; k < nvals ; k++)
    {
        // The kth ranked page is P[k].page (with k=0 being the highest rank),
        // and its pagerank is P[k].pagerank.
        P [k].pagerank = X [k] ;
        // I [k] == k will be true for SuiteSparse:GraphBLAS but in general I
        // can be returned in any order, so use I [k] instead of k, for other
        // GraphBLAS implementations.
        #ifdef GxB_SUITESPARSE_GRAPHBLAS
        P [k].page = k ;
        #else
        P [k].page = I [k] ;
        #endif
    }

    // free workspace
    FREEWORK ;

    // qsort (P) in descending order
    qsort (P, n, sizeof (LAGraph_PageRank), compar) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    tt = LAGraph_toc (tic) ;
    fprintf (stderr, "sort time %g\n", tt) ;

    (*Phandle) = P ;
    return (GrB_SUCCESS) ;
}

