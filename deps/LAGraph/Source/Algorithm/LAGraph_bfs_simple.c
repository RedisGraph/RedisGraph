//------------------------------------------------------------------------------
// LAGraph_bfs_simple:  simple breadth-first search
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

// LAGraph_bfs_simple:  based on the breadth-first search in the GraphBLAS C
// API Specification by Scott McMillan, CMU.  Modified by Tim Davis, Texas A&M.

// Perform a single-source BFS, starting at a source node s.  Returns a sparse
// vector v such that v(i) > 0 if node is reachable from s.  v(s)=1 and v(i)=k
// if the path with the fewest edges from s to i has k-1 edges.  If i is not
// reachable from s, then v(i) is implicitly zero and does not appear in the
// pattern of v.  That is, GrB_Vectors_nvals (&nreach,v) will return the
// size of the reach of s (including s itself).

// This method is a simple one for illustration only.  It can work well in
// practice, but its performance can be *very* poor in three important cases:

// (1) It takes Omega(n) time.  If nvals(v) << n is expected, use
// LAGraph_bfs_pushpull instead, which is much faster if v is expected to be
// very sparse.

// (2) It assumes the matrix A has no edges of explicit zero value.  The
// correct result will still be returned in this case, but it will be very
// slow.  See LAGraph_bfs_pushpull for a method that handles this case
// efficiently.

// (3) It assumes that vxm(q,A) is fast, and implemented in a 'push' fashion,
// using saxpy operations instead of dot products.  This requires that the rows
// A(i,:) are efficient to access, which is the case if A is in CSR format
// internally in GraphBLAS (or perhaps in both CSR and CSC formats).  This
// method will be *exceedlingly* slow if A is a MATLAB matrix in CSC format.

// See LAGraph_bfs_pushpull, which handles the above three cases.

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL    \
{                           \
    GrB_free (&v) ;         \
    GrB_free (&q) ;         \
}

GrB_Info LAGraph_bfs_simple     // push-only BFS
(
    GrB_Vector *v_output,   // v(i) is the BFS level of node i in the graph
    GrB_Matrix A,           // input graph, treated as if boolean in semiring
    GrB_Index s             // starting node of the BFS
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Vector q = NULL ;           // nodes visited at each level
    GrB_Vector v = NULL ;           // result vector
    if (v_output == NULL) LAGRAPH_ERROR ("argument missing", GrB_NULL_POINTER) ;
    GrB_Index n, nvals ;
    LAGRAPH_OK (GrB_Matrix_nrows (&n, A)) ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    // create an empty vector v, and make it dense
    LAGRAPH_OK (GrB_Vector_new (&v, (n > INT32_MAX) ? GrB_INT64:GrB_INT32, n)) ;
    LAGRAPH_OK (GrB_assign (v, NULL, NULL, 0, GrB_ALL, n, NULL)) ;
    LAGRAPH_OK (GrB_Vector_nvals (&n, v)) ;     // finish pending work on v

    // create a boolean vector q, and set q(s) to true
    LAGRAPH_OK (GrB_Vector_new (&q, GrB_BOOL, n)) ;
    LAGRAPH_OK (GrB_Vector_setElement (q, true, s)) ;

    //--------------------------------------------------------------------------
    // BFS traversal and label the nodes
    //--------------------------------------------------------------------------

    for (int64_t level = 1 ; level <= n ; level++)
    {
        // v<q> = level
        LAGRAPH_OK (GrB_assign (v, q, NULL, level, GrB_ALL, n, NULL)) ;

        // successor = ||(q)
        bool anyq ;
        LAGRAPH_OK (GrB_reduce (&anyq, NULL, LAGraph_LOR_MONOID, q, NULL)) ;
        if (!anyq) break ;

        // q'<!v> = q'*A
        LAGRAPH_OK (GrB_vxm (q, v, NULL, LAGraph_LOR_LAND_BOOL, q, A, 
            LAGraph_desc_oocr)) ;
    }

    //--------------------------------------------------------------------------
    // make v sparse
    //--------------------------------------------------------------------------

    LAGRAPH_OK (GrB_assign (v, v, NULL, v, GrB_ALL, n, LAGraph_desc_ooor)) ;
    GrB_Vector_nvals (&nvals, v) ;  // finish the work

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    (*v_output) = v ;       // return result
    v = NULL ;              // set to NULL so LAGRAPH_FREE_ALL doesn't free it
    LAGRAPH_FREE_ALL ;      // free all workspace (except for result v)
    return (GrB_SUCCESS) ;
}

