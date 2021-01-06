//------------------------------------------------------------------------------
// SuiteSparse/GraphBLAS/Demo/Source/ipagerank: pagerank using uint64 semiring
//------------------------------------------------------------------------------ 
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------ 
// A is a square unsymmetric binary matrix of size n-by-n, where A(i,j) is the
// edge (i,j).  Self-edges are OK.  A can be of any built-in type.

// On output, P is pointer to an array of iPageRank structs.  P[0] is the
// highest ranked page, with pagerank P[0].pagerank and the page corresponds to
// node number P[0].page in the graph.  P[1] is the next page, and so on, to
// the lowest-ranked page P[n-1].page with rank P[n-1].pagerank.

// See ipagerank.m for the equivalent computation in MATLAB (except the random
// number generator differs).

// This method uses no floating-point arithmetic at all.

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

// NOTE: this operator uses global value.  ipagerank can be done in
// parallel, internally, but only one instance of ipagerank can be used.
uint64_t ic ;

// scale by the integer ic
void iscale (uint64_t *z, const uint64_t *x)
{
    (*z) = ic * (*x) ;
}

// divide an integer by ZSCALE = 2^30, guard against integer underflow
void idiv (uint64_t *z, const uint64_t *x)
{
    (*z) = (*x) / ZSCALE ;
    if ((*z) == 0) (*z) = 1 ;
}

//------------------------------------------------------------------------------
// comparison function for qsort
//------------------------------------------------------------------------------

int icompar (const void *x, const void *y)
{
    iPageRank *a = (iPageRank *) x ;
    iPageRank *b = (iPageRank *) y ;

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
// ipagerank: compute the iPageRank of all nodes in a graph
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info ipagerank          // GrB_SUCCESS or error condition
(
    iPageRank **Phandle,    // output: pointer to array of iPageRank structs
    GrB_Matrix A            // input graph, not modified
)
{

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Info info ;
    uint64_t *X = NULL ;
    GrB_Index n, *I = NULL ;
    iPageRank *P = NULL ;
    GrB_Vector r = NULL ;
    GrB_UnaryOp op_scale = NULL, op_div = NULL ;
    GrB_Matrix C = NULL ;
    (*Phandle) = NULL ;

    // n = size (A,1) ;         // number of nodes
    OK (GrB_Matrix_nrows (&n, A)) ;

    // ic = 912680550 if ZSCALE = 2^30
    // double c = 0.85 ;           // probability of walking to random neighbor
    // ic = 0.85 * ZSCALE ;        // scaled integer version of c
    ic = 912680550 ;

    // Note the random number generate used here differs from MATLAB, so this
    // function will not compute exactly the same thing as ipagerank.m.

    // since rand() is in the range 0 to RAND_MAX, the sum of all unscaled
    // rand() values will be about n*RMAX/2.  The desired sum(r) = ZSCALE,
    // so scale each value from rand() by 2*ZSCALE / (RMAX*n)

    #define RMAX (((uint64_t) RAND_MAX) + 1)

    // r = rand (1,n) ;         // random initial pageranks
    srand ((int) n) ;
    OK (GrB_Vector_new (&r, GrB_UINT64, n)) ;
    for (int64_t i = 0 ; i < n ; i++)
    {
        // get a random integer in the range 0 to RMAX-1.
        uint64_t x = (uint64_t) rand ( ) ;

        // ZSCALE = 2^30 so 2*ZSCALE = 2^31.  RMAX is typically 2^31.  So the
        // ratio 2*ZSCALE / RMAX is typically equal to 1.  In that case,
        // neither of the two if-cases need to be used.  The test is a
        // compile-time constant so the compiler should be able to remove all
        // of if-test code below.  But do this anway in case RMAX is not 2^31.
        if (2*ZSCALE > RMAX)
        {
            x = x * (2*ZSCALE / RMAX) ;
        }
        else if (2*ZSCALE < RMAX)
        {
            // RMAX is larger than 2*ZSCALE, so instead of multiplying
            // by (2*ZSCALE / RMAX), divide by the reciprocal.
            x = x / (RMAX / 2*ZSCALE) ;
        }

        // finish the scaling by dividing by n
        x = x / n ;

        // ensure x > 0 however
        if (x == 0) x = 1 ;

        // now each r(i) is in the range 1 to 2*ZSCALE/n, and the expected value
        // of sum (r) will be ZSCALE.
        OK (GrB_Vector_setElement_UINT64 (r, x, i)) ;
    }

    // double a = (1-c) / n ;   // to jump to any random node in entire graph

    // ZSCALE - ic = 161061274 if ZSCALE = 2^31
    uint64_t ia = ZSCALE - ic ;      // scaled integer version of (1-c)
    ia = ia / n ;
    if (ia == 0) ia = 1 ;       // ensure ia > 0

    OK (irowscale (&C, A)) ;    // C = scale A by out-degree

    // create unary operators
    OK (GrB_UnaryOp_new (&op_scale,
        (GxB_unary_function) iscale, GrB_UINT64, GrB_UINT64)) ;
    OK (GrB_UnaryOp_new (&op_div,
        (GxB_unary_function) idiv,   GrB_UINT64, GrB_UINT64)) ;

    //--------------------------------------------------------------------------
    // iterate to compute the pagerank of each node
    //--------------------------------------------------------------------------

    for (int i = 0 ; i < 20 ; i++)
    {
        // r = floor ((floor (floor ((c*r)/z) * C) + floor (a * sum (r))) / z) ;

        // with implicit floor:
        // r = ((((c*r) / z) * C) + (a * sum (r))) / z ;

        // s = ia * sum (r) ;
        uint64_t s ;
        OK (GrB_Vector_reduce_UINT64 (&s, NULL, GrB_PLUS_MONOID_UINT64,
            r, NULL)) ;
        s = s * ia ;

        // r = ic * r
        OK (GrB_Vector_apply (r, NULL, NULL, op_scale, r, NULL)) ;

        // r = r / ZSCALE
        OK (GrB_Vector_apply (r, NULL, NULL, op_div, r, NULL)) ;

        // r = r * C
        OK (GrB_vxm (r, NULL, NULL, GxB_PLUS_TIMES_UINT64, r, C, NULL)) ;

        // r = r + s
        OK (GrB_Vector_assign_FP64 (r, NULL, GrB_PLUS_UINT64, s,
            GrB_ALL, n, NULL)) ;

        // r = r / ZSCALE
        OK (GrB_Vector_apply (r, NULL, NULL, op_div, r, NULL)) ;
    }

    //--------------------------------------------------------------------------
    // sort the nodes by pagerank
    //--------------------------------------------------------------------------

    // [r,irank] = sort (r, 'descend') ;

    // [I,X] = find (r) ;
    X = (uint64_t *) malloc (n * sizeof (uint64_t)) ;
    I = (GrB_Index *) malloc (n * sizeof (GrB_Index)) ;
    CHECK (I != NULL && X != NULL, GrB_OUT_OF_MEMORY) ;
    GrB_Index nvals = n ;
    OK (GrB_Vector_extractTuples_UINT64 (I, X, &nvals, r)) ;

    // this will always be true since r is dense, but double-check anyway:
    CHECK (nvals == n, GrB_INVALID_VALUE) ;

    // r no longer needed
    GrB_Vector_free (&r) ;

    // P = struct (X,I)
    P = (iPageRank *) malloc (n * sizeof (iPageRank)) ;
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
    qsort (P, n, sizeof (iPageRank), icompar) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*Phandle) = P ;
    return (GrB_SUCCESS) ;
}

