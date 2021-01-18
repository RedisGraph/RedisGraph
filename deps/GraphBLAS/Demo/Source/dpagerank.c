//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Demo/Source/dpagerank: pagerank using a real semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A is a square unsymmetric binary matrix of size n-by-n, where A(i,j) is the
// edge (i,j).  Self-edges are OK.  A can be of any built-in type.

// On output, P is pointer to an array of PageRank structs.  P[0] is the
// highest ranked page, with pagerank P[0].pagerank and the page corresponds to
// node number P[0].page in the graph.  P[1] is the next page, and so on, to
// the lowest-ranked page P[n-1].page with rank P[n-1].pagerank.

// See dpagerank.m for the equivalent computation in MATLAB (except the random
// number generator differs).

#include "GraphBLAS.h"

//------------------------------------------------------------------------------
// helper macros
//------------------------------------------------------------------------------

// free all workspace
#define FREEWORK                        \
{                                       \
    GrB_Matrix_free (&C) ;              \
    GrB_Vector_free (&r) ;              \
    if (I != NULL) free (I) ;           \
    if (X != NULL) free (X) ;           \
    GrB_UnaryOp_free (&op_scale) ;      \
    GrB_UnaryOp_free (&op_div) ;        \
}

// error handler: free output P and all workspace (used by CHECK and OK macros)
#define FREE_ALL                \
{                               \
    if (P != NULL) free (P) ;   \
    FREEWORK ;                  \
}

#undef GB_PUBLIC
#define GB_LIBRARY
#include "graphblas_demos.h"

//------------------------------------------------------------------------------
// scalar operators
//------------------------------------------------------------------------------

// NOTE: these operators use global values.  dpagerank can be done in parallel,
// internally, but only one instance of dpagerank can be used.
double c, s ;
void fscale (double *z, const double *x) { (*z) = c * (*x) ; }
void fdiv   (double *z, const double *x) { (*z) = (*x) / s ; }

//------------------------------------------------------------------------------
// comparison function for qsort
//------------------------------------------------------------------------------

int compar (const void *x, const void *y)
{
    PageRank *a = (PageRank *) x ;
    PageRank *b = (PageRank *) y ;

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
// dpagerank: compute the PageRank of all nodes in a graph
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info dpagerank          // GrB_SUCCESS or error condition
(
    PageRank **Phandle,     // output: pointer to array of PageRank structs
    GrB_Matrix A            // input graph, not modified
)
{

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Info info ;
    double *X = NULL ;
    GrB_Index n, *I = NULL ;
    PageRank *P = NULL ;
    GrB_Vector r = NULL ;
    GrB_UnaryOp op_scale = NULL, op_div = NULL ;
    GrB_Matrix C = NULL ;
    (*Phandle) = NULL ;

    // n = size (A,1) ;         // number of nodes
    OK (GrB_Matrix_nrows (&n, A)) ;

    c = 0.85 ;                  // probability of walking to random neighbor

    // Note the random number generate used here differs from MATLAB, so this
    // function will not compute exactly the same thing as dpagerank.m.

    // r = rand (1,n) ;         // random initial pageranks
    // simple_rand_seed ((uint64_t) n) ;
    srand ((int) n) ;
    OK (GrB_Vector_new (&r, GrB_FP64, n)) ;
    for (int64_t i = 0 ; i < n ; i++)
    {
        // get a random double value in the range 0 to 1
        // this is too low quality:
        // double x = simple_rand_x ( ) ;
        // this is not portable:
        double x = ((double) rand ( )) / (double) RAND_MAX ;
        OK (GrB_Vector_setElement_FP64 (r, x, i)) ;
    }

    // skip this (see dpagerank.m and compare with ipagerank.m):
    // r = r / sum (r) ;        // normalize so sum(r) == 1

    double a = (1-c) / n ;      // to jump to any random node in entire graph

    OK (drowscale (&C, A)) ;    // C = scale A by out-degree

    // create unary operators
    OK (GrB_UnaryOp_new (&op_scale,
        (GxB_unary_function) fscale, GrB_FP64, GrB_FP64)) ;
    OK (GrB_UnaryOp_new (&op_div  ,
        (GxB_unary_function) fdiv,   GrB_FP64, GrB_FP64)) ;

    //--------------------------------------------------------------------------
    // iterate to compute the pagerank of each node
    //--------------------------------------------------------------------------

    for (int i = 0 ; i < 20 ; i++)
    {
        // r = ((c*r) * C) + (a * sum (r)) ;

        // s = a * sum (r) ;
        OK (GrB_Vector_reduce_FP64 (&s, NULL, GrB_PLUS_MONOID_FP64, r, NULL)) ;
        s *= a ;

        // r = c * r
        OK (GrB_Vector_apply (r, NULL, NULL, op_scale, r, NULL)) ;

        // r = r * C
        OK (GrB_vxm (r, NULL, NULL, GxB_PLUS_TIMES_FP64, r, C, NULL)) ;

        // r = r + s
        OK (GrB_Vector_assign_FP64 (r, NULL, GrB_PLUS_FP64, s,
            GrB_ALL, n, NULL)) ;
    }

    //--------------------------------------------------------------------------
    // scale the result
    //--------------------------------------------------------------------------

    // s = sum (r)
    OK (GrB_Vector_reduce_FP64 (&s, NULL, GrB_PLUS_MONOID_FP64, r, NULL)) ;

    // r = r / s
    OK (GrB_Vector_apply (r, NULL, NULL, op_div, r, NULL)) ;

    //--------------------------------------------------------------------------
    // sort the nodes by pagerank
    //--------------------------------------------------------------------------

    // [r,irank] = sort (r, 'descend') ;

    // [I,X] = find (r) ;
    X = (double *) malloc (n * sizeof (double)) ;
    I = (GrB_Index *) malloc (n * sizeof (GrB_Index)) ;
    CHECK (I != NULL && X != NULL, GrB_OUT_OF_MEMORY) ;
    GrB_Index nvals = n ;
    OK (GrB_Vector_extractTuples_FP64 (I, X, &nvals, r)) ;

    // this will always be true since r is dense, but double-check anyway:
    CHECK (nvals == n, GrB_INVALID_VALUE) ;

    // r no longer needed
    GrB_Vector_free (&r) ;

    // P = struct (X,I)
    P = (PageRank *) malloc (n * sizeof (PageRank)) ;
    CHECK (P != NULL, GrB_OUT_OF_MEMORY) ;
    for (int64_t k = 0 ; k < nvals ; k++)
    {
        // The kth ranked page is P[k].page (with k=0 being the highest rank),
        // and its pagerank is P[k].pagerank.
        P [k].pagerank = X [k] ;
        // I [k] == k will be true for SuiteSparse:GraphBLAS but in general I
        // can be returned in any order, so use I [k] instead of k, for other
        // GraphBLAS implementations.
        P [k].page = I [k] ;
    }

    // free workspace
    FREEWORK ;

    // qsort (P) in descending order
    qsort (P, n, sizeof (PageRank), compar) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*Phandle) = P ;
    return (GrB_SUCCESS) ;
}

