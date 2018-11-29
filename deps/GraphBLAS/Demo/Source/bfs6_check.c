//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/bfs6_check.c: breadth first search using vxm
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

#define FREE_ALL                \
    GrB_free (&v) ;             \
    GrB_free (&q) ;             \
    GrB_free (&desc) ;          \
    GrB_free (&apply_level) ;

#include "demos.h"

//------------------------------------------------------------------------------
// bfs6: breadth first search using a Boolean semiring
//------------------------------------------------------------------------------

// Given a n x n adjacency matrix A and a source node s, performs a BFS
// traversal of the graph and sets v[i] to the level in which node i is
// visited (v[s] == 1).  If i is not reacheable from s, then v[i] = 0. (Vector
// v should be empty on input.)  The graph A need not be Boolean on input;
// if it isn't Boolean, the semiring will properly typecast it to Boolean.

GrB_Info bfs6_check         // BFS of a graph (using unary operator)
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
    GrB_Index n ;                               // # of nodes in the graph
    GrB_Vector q = NULL ;                       // nodes visited at each level
    GrB_Vector v = NULL ;                       // result vector
    GrB_Descriptor desc = NULL ;                // Descriptor for vxm
    GrB_UnaryOp apply_level = NULL ;            // unary op: z = f(x) = level

    OK (GrB_Matrix_nrows (&n, A)) ;             // n = # of rows of A
    OK (GrB_Vector_new (&v, GrB_INT32, n)) ;    // Vector<int32_t> v(n) = 0
    OK (GrB_Vector_new (&q, GrB_BOOL, n)) ;     // Vector<bool> q(n) = false
    OK (GrB_Vector_setElement (q, true, s)) ;   // q[s] = true, false elsewhere

    // descriptor: invert the mask for vxm, and clear output before assignment
    OK (GrB_Descriptor_new (&desc)) ;
    OK (GxB_set (desc, GrB_MASK, GrB_SCMP)) ;
    OK (GxB_set (desc, GrB_OUTP, GrB_REPLACE)) ;

    // create a unary operator
    OK (GrB_UnaryOp_new (&apply_level, bfs_level, GrB_INT32, GrB_BOOL)) ;

    //--------------------------------------------------------------------------
    // BFS traversal and label the nodes
    //--------------------------------------------------------------------------

    GrB_Index nvals = 1 ;
    for (level = 1 ; nvals > 0 && level <= n ; level++)
    {
        // v[q] = level, using apply.  This function applies the unary operator
        // to the entries in q, which are the unvisited successors, and then
        // writes their levels to v, thus updating the levels of those nodes in
        // v.  The patterns of v and q are disjoint.
        OK (GrB_apply (v, NULL, GrB_PLUS_INT32, apply_level, q, NULL)) ;

        // q<!v> = A ||.&& q ; finds all the unvisited successors from current
        // q, using !v as the mask
        OK (GrB_vxm (q, v, NULL, GxB_LOR_LAND_BOOL, q, A, desc)) ;

        GrB_Vector_nvals (&nvals, q) ;
    }

    *v_output = v ;         // return result
    v = NULL ;              // set to NULL so FREE_ALL doesn't free it

    FREE_ALL ;              // free all workspace

    return (GrB_SUCCESS) ;
}

