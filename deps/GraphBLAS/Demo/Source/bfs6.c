//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/bfs6.c: breadth first search (vxm and apply)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

// No copyright claim is made for this particular file; the above copyright
// applies to all of SuiteSparse:GraphBLAS, not this file.

// This method has been updated as of Version 2.2 of SuiteSparse:GraphBLAS.
// It now assumes the matrix is held by row (GxB_BY_ROW) and uses GrB_vxm
// instead of GrB_mxv.  It now more closely matches the BFS example in the
// GraphBLAS C API Specification.

// A must not have any explicit zeros.

// NOTE: this method can be *slow*, in special cases (v very sparse on output,
// A in CSC format instead of the default CSR, or if A has any explicit values
// equal to zero in its pattern).  See LAGraph_bfs_pushpull for a faster method
// that handles these cases.  Do not benchmark this code!  It is just for
// simple illustration.  Use the LAGraph_bfs_pushpull for benchmarking and
// production use.

#include "GraphBLAS.h"
#undef GB_PUBLIC
#define GB_LIBRARY
#include "graphblas_demos.h"

//------------------------------------------------------------------------------
// bfs_level: for unary operator
//------------------------------------------------------------------------------

// level = depth in BFS traversal, roots=1, unvisited=0.

// Note the operator accesses a global variable outside the control of
// GraphBLAS.  This is safe, but care must be taken not to change the global
// variable "level" while pending operations have yet to be completed.

int32_t bfs_level_global = 0 ;

void bfs_level (void *result, const void *element)
{
    // Note this function does not depend on its input.  It returns the value
    // of the global variable level for all inputs.  It is applied to the
    // vector q via GrB_apply, which only applies the unary operator to entries
    // in the pattern.  Entries not in the pattern remain implicit (zero in
    // this case), and then are not added by the GrB_PLUS_INT32 accum function.
    (* ((int32_t *) result)) = bfs_level_global ;
}

//------------------------------------------------------------------------------
// bfs6: breadth first search using a Boolean semiring
//------------------------------------------------------------------------------

// Given a n x n adjacency matrix A and a source node s, performs a BFS
// traversal of the graph and sets v[i] to the level in which node i is
// visited (v[s] == 1).  If i is not reacheable from s, then v[i] = 0. (Vector
// v should be empty on input.)  The graph A need not be Boolean on input;
// if it isn't Boolean, the semiring will properly typecast it to Boolean.

GB_PUBLIC
GrB_Info bfs6               // BFS of a graph (using apply)
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
    GrB_Descriptor desc = NULL ;           // Descriptor for vxm
    GrB_UnaryOp apply_level = NULL ;       // unary op:
                                           // z = f(x) = bfs_level_global

    GrB_Matrix_nrows (&n, A) ;             // n = # of rows of A
    GrB_Vector_new (&v, GrB_INT32, n) ;    // Vector<int32_t> v(n) = 0
    GrB_Vector_assign_INT32 (v, NULL, NULL, 0, GrB_ALL, n, NULL) ; // make dense
    GrB_Vector_nvals (&n, v) ;             // finish pending work on v

    GrB_Vector_new (&q, GrB_BOOL, n) ;     // Vector<bool> q(n) = false
    GrB_Vector_setElement_BOOL (q, true, s) ;   // q[s] = true, false elsewhere

    GrB_Descriptor_new (&desc) ;
    GrB_Descriptor_set (desc, GrB_MASK, GrB_COMP) ;     // invert the mask
    GrB_Descriptor_set (desc, GrB_OUTP, GrB_REPLACE) ;  // clear q first

    // create a unary operator
    GrB_UnaryOp_new (&apply_level, bfs_level, GrB_INT32, GrB_BOOL) ;

    //--------------------------------------------------------------------------
    // BFS traversal and label the nodes
    //--------------------------------------------------------------------------

    bool successor = true ; // true when some successor found
    for (bfs_level_global = 1 ; successor && bfs_level_global <= n ; 
         bfs_level_global++)
    {

        // v[q] = bfs_level_global, using apply.  This function applies the
        // unary operator to the entries in q, which are the unvisited
        // successors, and then writes their levels to v, thus updating the
        // levels of those nodes in v.  The patterns of v and q are disjoint.
        GrB_Vector_apply (v, NULL, GrB_PLUS_INT32, apply_level, q, NULL) ;

        // q<!v> = q ||.&& A ; finds all the unvisited
        // successors from current q, using !v as the mask
        GrB_vxm (q, v, NULL, GrB_LOR_LAND_SEMIRING_BOOL, q, A, desc) ;

        // successor = ||(q)
        GrB_Vector_reduce_BOOL (&successor, NULL, GrB_LOR_MONOID_BOOL, q, NULL) ;
    }

    // make v sparse
    GrB_Descriptor_set (desc, GrB_MASK, GxB_DEFAULT) ;  // mask not inverted
    GrB_Vector_assign (v, v, NULL, v, GrB_ALL, n, desc) ;

    *v_output = v ;         // return result

    GrB_Vector_free (&q) ;
    GrB_Descriptor_free (&desc) ;
    GrB_UnaryOp_free (&apply_level) ;

    return (GrB_SUCCESS) ;
}

