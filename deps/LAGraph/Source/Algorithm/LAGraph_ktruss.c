//------------------------------------------------------------------------------
// LAGraph_ktruss: k-truss subgraph
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

// LAGraph_ktruss: k-truss subgraph, contributed by Tim Davis, Texas A&M.

// Given a symmetric graph A with no-self edges, ktruss_graphblas finds the
// k-truss subgraph of A.

// TODO add sanitize step to remove diagonal

// TODO:  currently relies on a GxB* function in SuiteSparse/GraphBLAS.
// use #ifdef for non-SuiteSparse GraphBLAS libraries.

// The edge weights of A are treated as binary.  Explicit zero entries in A are
// treated as non-edges.  Any type will work, but uint32 is recommended for
// fastest results since that is the type used here for the semiring.
// GraphBLAS will do typecasting internally, but that takes extra time. 

// The output matrix C is the k-truss subgraph of A.  Its edges are a subset of
// A.  Each edge in C is part of at least k-2 triangles in C.  The pattern of C
// is the adjacency matrix of the k-truss subgraph of A.  The edge weights of C
// are the support of each edge.  That is, C(i,j)=nt if the edge (i,j) is part
// of nt triangles in C.  All edges in C have support of at least k-2.  The
// total number of triangles in C is sum(C)/6.  The number of edges in C is
// nnz(C)/2.  C is returned as symmetric with a zero-free diagonal.

// Usage: constructs C as the k-truss of A
//      GrB_Matrix C = NULL ;
//      int32_t nsteps ;
//      GrB_Info info = LAGraph_ktruss (&C, A, k, &nsteps) ;

// Compare this function with the MATLAB equivalent, ktruss.m, in
// LAGraph/Test/KTruss.  This function is derived from SuiteSparse/GraphBLAS/
// Extras/ktruss_graphblas.c.

// TODO add cite

#define LAGRAPH_FREE_ALL                \
    GrB_free (&Support) ;               \
    GrB_free (&C) ;

#include "LAGraph_internal.h"

//------------------------------------------------------------------------------
// C = ktruss_graphblas (A,k): find the k-truss subgraph of a graph
//------------------------------------------------------------------------------

GrB_Info LAGraph_ktruss         // compute the k-truss of a graph
(
    GrB_Matrix *Chandle,        // output k-truss subgraph, C
    const GrB_Matrix A,         // input adjacency matrix, A, not modified
    const uint32_t k,           // find the k-truss, where k >= 3
    int32_t *p_nsteps           // # of steps taken (ignored if NULL)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // ensure k is 3 or more
    if (k < 3) return (GrB_INVALID_VALUE) ;

    if (Chandle == NULL) return (GrB_NULL_POINTER) ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Info info ;

    GrB_Index n ;
    GrB_Matrix C = NULL ;
    GrB_Vector Support = NULL ;
    LAGRAPH_OK (GrB_Matrix_nrows (&n, A)) ;
    LAGRAPH_OK (GrB_Matrix_new (&C, GrB_UINT32, n, n)) ;

    // for the select operator
    uint32_t support = (k-2) ;
    LAGRAPH_OK (GrB_Vector_new (&Support, GrB_UINT32, 1)) ;
    LAGRAPH_OK (GrB_Vector_setElement (Support, support, 0)) ;
    GrB_Index ignore ;
    LAGRAPH_OK (GrB_Vector_nvals (&ignore, Support)) ;

    // last_cnz = nnz (A)
    GrB_Index cnz, last_cnz ;
    LAGRAPH_OK (GrB_Matrix_nvals (&last_cnz, A)) ;

    //--------------------------------------------------------------------------
    // find the k-truss of A
    //--------------------------------------------------------------------------

    for (int32_t nsteps = 1 ; ; nsteps++)
    {

        //----------------------------------------------------------------------
        // C<C> = C*C
        //----------------------------------------------------------------------

        GrB_Matrix C2 = (nsteps == 1) ? A : C ;
        LAGRAPH_OK (GrB_mxm (C, C2, NULL, GxB_PLUS_LAND_UINT32, C2, C2, NULL)) ;

        //----------------------------------------------------------------------
        // C = C .* (C >= support)
        //----------------------------------------------------------------------

        LAGRAPH_OK (GxB_select (C, NULL, NULL, LAGraph_support, C,
            #if GxB_IMPLEMENTATION >= GxB_VERSION (3,0,0)
            Support,    // V3.0.0 and later uses a GrB_Vector
            #else
            &support,   // V2.x and earlier uses a (const void *) pointer
            #endif
            NULL)) ;

        //----------------------------------------------------------------------
        // check if the k-truss has been found
        //----------------------------------------------------------------------

        LAGRAPH_OK (GrB_Matrix_nvals (&cnz, C)) ;
        if (cnz == last_cnz)
        {
            (*Chandle) = C ;                        // return the result C
            if (p_nsteps != NULL)
            {
                (*p_nsteps) = nsteps ;              // return # of steps
            }
            GrB_free (&Support) ;
            return (GrB_SUCCESS) ;
        }
        last_cnz = cnz ;
    }
}

