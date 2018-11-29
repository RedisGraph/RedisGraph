//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/bfs5m.c: breadth first search (vxm and assign/reduce)
//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

// This method has been updated as of Version 2.2 of SuiteSparse:GraphBLAS.
// It now assumes the matrix is held by row (GxB_BY_ROW) and uses GrB_vxm
// instead of GrB_mxv.  It now more closely matches the BFS example in the
// GraphBLAS C API Specification.

#include "demos.h"

//------------------------------------------------------------------------------
// bfs5m: breadth first search using a Boolean semiring
//------------------------------------------------------------------------------

// Given a n x n adjacency matrix A and a source node s, performs a BFS
// traversal of the graph and sets v[i] to the level in which node i is
// visited (v[s] == 1).  If i is not reacheable from s, then v[i] = 0. (Vector
// v should be empty on input.)  The graph A need not be Boolean on input;
// if it isn't Boolean, the semiring will properly typecast it to Boolean.

GrB_Info bfs5m              // BFS of a graph (using vector assign & reduce)
(
    GrB_Vector *v_output,   // v [i] is the BFS level of node i in the graph
    const GrB_Matrix A,     // input graph, treated as if boolean in semiring
    GrB_Index s             // starting node of the BFS
)
{

    //--------------------------------------------------------------------------
    // set up the semiring and initialize the vector v
    //--------------------------------------------------------------------------

    GrB_Index n ;                          // # of nodes in the graph
    GrB_Vector q = NULL ;                  // nodes visited at each level
    GrB_Vector v = NULL ;                  // result vector
    GrB_Monoid Lor = NULL ;                // Logical-or monoid
    GrB_Semiring Boolean = NULL ;          // Boolean semiring
    GrB_Descriptor desc = NULL ;           // Descriptor for vxm

    GrB_Matrix_nrows (&n, A) ;             // n = # of rows of A
    GrB_Vector_new (&v, GrB_INT32, n) ;    // Vector<int32_t> v(n) = 0
    // This is a little faster if the whole graph is expected to be searched,
    // but slower if only a small part of the graph is reached:
    // for (int32_t i = 0 ; i < n ; i++) GrB_Vector_setElement (v, 0, i) ;
    GrB_Vector_new (&q, GrB_BOOL, n) ;     // Vector<bool> q(n) = false
    GrB_Vector_setElement (q, true, s) ;   // q[s] = true, false elsewhere

    GrB_Monoid_new (&Lor, GrB_LOR, (bool) false) ;
    GrB_Semiring_new (&Boolean, Lor, GrB_LAND) ;
    GrB_Descriptor_new (&desc) ;
    GrB_Descriptor_set (desc, GrB_MASK, GrB_SCMP) ;     // invert the mask
    GrB_Descriptor_set (desc, GrB_OUTP, GrB_REPLACE) ;  // clear q first

    //--------------------------------------------------------------------------
    // BFS traversal and label the nodes
    //--------------------------------------------------------------------------

    bool successor = true ; // true when some successor found
    for (int32_t level = 1 ; successor && level <= n ; level++)
    {
        // v<q> = level, using vector assign with q as the mask
        GrB_assign (v, q, NULL, level, GrB_ALL, n, NULL) ;

        // q<!v> = q ||.&& A ; finds all the unvisited
        // successors from current q, using !v as the mask
        GrB_vxm (q, v, NULL, Boolean, q, A, desc) ;

        // successor = ||(q)
        GrB_reduce (&successor, NULL, Lor, q, NULL) ;
    }

    *v_output = v ;         // return result

    GrB_free (&q) ;
    GrB_free (&Lor) ;
    GrB_free (&Boolean) ;
    GrB_free (&desc) ;

    return (GrB_SUCCESS) ;
}

