//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/bfs5m_check.c: BFS with vxm and assign/reduce
//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

// This method has been updated as of Version 2.2 of SuiteSparse:GraphBLAS.
// It now assumes the matrix is held by row (GxB_BY_ROW) and uses GrB_vxm
// instead of GrB_mxv.  It now more closely matches the BFS example in the
// GraphBLAS C API Specification.

// This version uses a predefined semiring (GxB_LOR_LAND_BOOL) and a predefined
// monoid (GxB_LOR_BOOL_MONOID), in GraphBLAS.h.  It also checks the status of
// each call to GraphBLAS functions.  These two changes are unrelated.  Both
// change are made here to illustrate two different things.

// "OK(x)" macro calls a GraphBLAS method, and if it fails, prints the error,
// frees workspace, and returns to the caller.  It uses the FREE_ALL macro
// to free the workspace

#define FREE_ALL            \
    GrB_free (&v) ;         \
    GrB_free (&q) ;         \
    GrB_free (&desc) ;

#include "demos.h"

//------------------------------------------------------------------------------
// bfs5m: breadth first search using a Boolean semiring
//------------------------------------------------------------------------------

// Given a n x n adjacency matrix A and a source node s, performs a BFS
// traversal of the graph and sets v[i] to the level in which node i is
// visited (v[s] == 1).  If i is not reacheable from s, then v[i] = 0. (Vector
// v should be empty on input.)  The graph A need not be Boolean on input;
// if it isn't Boolean, the semiring will properly typecast it to Boolean.

GrB_Info bfs5m_check        // BFS of a graph (using vector assign & reduce)
(
    GrB_Vector *v_output,   // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    GrB_Index s             // starting node of the BFS
)
{

    //--------------------------------------------------------------------------
    // set up the semiring and initialize the vector v
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Index n ;                          // # of nodes in the graph
    GrB_Vector q = NULL ;                  // nodes visited at each level
    GrB_Vector v = NULL ;                  // result vector
    GrB_Descriptor desc = NULL ;           // Descriptor for vxm

    OK (GrB_Matrix_nrows (&n, A)) ;             // n = # of rows of A
    OK (GrB_Vector_new (&v, GrB_INT32, n)) ;    // Vector<int32_t> v(n) = 0
    // This is a little faster if the whole graph is expected to be searched:
    // but slower if only a small part of the graph is reached:
    // for (int32_t i = 0 ; i < n ; i++) OK (GrB_Vector_setElement (v, 0, i)) ;
    OK (GrB_Vector_new (&q, GrB_BOOL, n)) ;     // Vector<bool> q(n) = false
    OK (GrB_Vector_setElement (q, true, s)) ;   // q[s] = true, false elsewhere

    // descriptor: invert the mask for vxm, and clear output before assignment
    OK (GrB_Descriptor_new (&desc)) ;
    OK (GxB_set (desc, GrB_MASK, GrB_SCMP)) ;
    OK (GxB_set (desc, GrB_OUTP, GrB_REPLACE)) ;

    //--------------------------------------------------------------------------
    // BFS traversal and label the nodes
    //--------------------------------------------------------------------------

    bool successor = true ; // true when some successor found
    for (int32_t level = 1 ; successor && level <= n ; level++)
    {
        // v<q> = level, using vector assign with q as the mask
        OK (GrB_assign (v, q, NULL, level, GrB_ALL, n, NULL)) ;

        // q<!v> = q ||.&& A ; finds all the unvisited
        // successors from current q, using !v as the mask
        OK (GrB_vxm (q, v, NULL, GxB_LOR_LAND_BOOL, q, A, desc)) ;

        // successor = ||(q)
        OK (GrB_reduce (&successor, NULL, GxB_LOR_BOOL_MONOID, q, NULL)) ;
    }

    *v_output = v ;         // return result
    v = NULL ;              // set to NULL so FREE_ALL doesn't free it

    FREE_ALL ;              // free all workspace

    return (GrB_SUCCESS) ;
}

