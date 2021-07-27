//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/mis.c: maximal independent set
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

// No copyright claim is made for this particular file; the above copyright
// applies to all of SuiteSparse:GraphBLAS, not this file.

// FUTURE: revise this method and add it to LAGraph

#include "GraphBLAS.h"
#undef GB_PUBLIC
#define GB_LIBRARY
#include "graphblas_demos.h"

//------------------------------------------------------------------------------
// mis: maximal independent set
//------------------------------------------------------------------------------

// A variant of Luby's randomized algorithm [Luby 1985]. 

// Given a numeric n x n adjacency matrix A of an unweighted and undirected
// graph (where the value true represents an edge), compute a maximal set of
// independent nodes and return it in a boolean n-vector, 'iset' where
// set[i] == true implies node i is a member of the set (the iset vector
// should be uninitialized on input.)

// The graph cannot have any self edges, and it must be symmetric.  These
// conditions are not checked.  Self-edges (diagonal entries) will cause the
// method to stall.

// Singletons require special treatment.  Since they have no neighbors, their
// prob is never greater than the max of their neighbors, so they never get
// selected and cause the method to stall.  To avoid this case they are removed
// from the candidate set at the begining, and added to the iset.

GB_PUBLIC
GrB_Info mis                    // compute a maximal independent set
(
    GrB_Vector *iset_output,    // iset(i) = true if i is in the set
    const GrB_Matrix A,         // symmetric Boolean matrix
    int64_t seed                // random number seed
)
{
    printf ("MIS demo: this code old, and slower than it will be once the\n") ;
    printf ("TODOs are fixed below.  MIS will be added to LAGraph and\n") ;
    printf ("removed from the GraphBLAS/Demo folder.\n") ;

    GrB_Vector iset = NULL ;
    GrB_Vector prob = NULL ;            // random probability for each node
    GrB_Vector neighbor_max = NULL ;    // value of max neighbor probability
    GrB_Vector new_members = NULL ;     // set of new members to iset
    GrB_Vector new_neighbors = NULL ;   // new neighbors to new iset members
    GrB_Vector candidates = NULL ;      // candidate members to iset
    GrB_Semiring maxSelect1st = NULL ;  // Max/Select1st "semiring"
    GrB_BinaryOp set_random = NULL ;
    GrB_Vector degrees = NULL ;

    GrB_Index n ;

    GrB_Matrix_nrows (&n, A) ;                 // n = # of nodes in graph

    GrB_Vector_new (&prob, GrB_FP64, n) ;
    GrB_Vector_new (&neighbor_max, GrB_FP64, n) ;
    GrB_Vector_new (&new_members, GrB_BOOL, n) ;
    GrB_Vector_new (&new_neighbors, GrB_BOOL, n) ;
    GrB_Vector_new (&candidates, GrB_BOOL, n) ;

    // Initialize independent set vector, bool
    GrB_Vector_new (&iset, GrB_BOOL, n) ;

    // create the maxSelect1st semiring
    GrB_Semiring_new (&maxSelect1st, GrB_MAX_MONOID_FP64, GrB_FIRST_FP64) ;

    // create the random number seeds
    GrB_Vector Seed, X ;
    prand_init ( ) ;
    prand_seed (&Seed, seed, n, 0) ;
    GrB_Vector_new (&X, GrB_FP64, n) ;

    // create the mis_score binary operator
    GrB_BinaryOp_new (&set_random, mis_score2, GrB_FP64, GrB_UINT32, GrB_FP64) ;

    // compute the degree of each nodes
    GrB_Vector_new (&degrees, GrB_FP64, n) ;
    // TODO: use GrB_mxv with v iso full, semiring GrB_PLUS_PAIR_FP64
    GrB_Matrix_reduce_Monoid (degrees, NULL, NULL, GrB_PLUS_MONOID_FP64,
        A, NULL) ;

    // singletons are not candidates; they are added to iset first instead
    // candidates[degree != 0] = 1
    // TODO: use GrB_DESC_S
    GrB_Vector_assign_BOOL (candidates, degrees, NULL, true, GrB_ALL, n, NULL); 

    // add all singletons to iset
    // iset[degree == 0] = 1
    // TODO: use GrB_DESC_RSC, but only do this if nvals(degree) < n
    GrB_Vector_assign_BOOL (iset, degrees, NULL, true, GrB_ALL, n, GrB_DESC_RC);

    // Iterate while there are candidates to check.
    GrB_Index nvals ;
    GrB_Vector_nvals (&nvals, candidates) ;

    int64_t last_nvals = nvals ;

    while (nvals > 0)
    {
        // sparsify the random number seeds (just keep it for each candidate) 
        GrB_Vector_assign (Seed, candidates, NULL, Seed, GrB_ALL, n,
            GrB_DESC_R) ;

        // compute a random probability scaled by inverse of degree
        // GrB_Vector_apply (prob, candidates, NULL, set_random, degrees,
        //      GrB_DESC_R) ;
        prand_xget (X, Seed) ;
        GrB_Vector_eWiseMult_BinaryOp (prob, candidates, NULL, set_random,
            degrees, X, GrB_DESC_R) ;   // TODO: use GrB_DESC_RS

        // compute the max probability of all neighbors
        GrB_vxm (neighbor_max, candidates, NULL, maxSelect1st,
            prob, A, GrB_DESC_R) ;      // TODO: use GrB_DESC_RS

        // select node if its probability is > than all its active neighbors
        GrB_Vector_eWiseAdd_BinaryOp (new_members, NULL, NULL, GrB_GT_FP64,
            prob, neighbor_max, NULL) ;

        // add new members to independent set.
        GrB_Vector_eWiseAdd_BinaryOp (iset, NULL, NULL, GrB_LOR, iset,
            new_members, NULL) ;

        // remove new members from set of candidates
        // candidates<!new_members> = candidates
        GrB_Vector_apply (candidates, new_members, NULL, GrB_IDENTITY_BOOL,
            candidates, GrB_DESC_RC) ;      // TODO: use GrB_DESC_RSC

        GrB_Vector_nvals (&nvals, candidates) ;
        if (nvals == 0) { break ; }                  // early exit condition

        // Neighbors of new members can also be removed from candidates
        // TODO: use ANY_PAIR?
        GrB_vxm (new_neighbors, candidates, NULL, GrB_LOR_LAND_SEMIRING_BOOL,
            new_members, A, NULL) ;         // TODO: use GrB_DESC_S
        // candidates<!new_neighbors> = candidates
        GrB_Vector_apply (candidates, new_neighbors, NULL, GrB_IDENTITY_BOOL,
            candidates, GrB_DESC_RC) ;      // TODO: use GrB_DESC_RSC?

        GrB_Vector_nvals (&nvals, candidates) ;

        // this will not occur, unless the input is corrupted somehow
        if (last_nvals == nvals) { printf ("stall!\n") ; exit (1) ; }
        last_nvals = nvals ;
    }

    // drop explicit false values
    GrB_Vector_apply (iset, iset, NULL, GrB_IDENTITY_BOOL, iset, GrB_DESC_R) ;

    // return result
    *iset_output = iset ;

    // free workspace
    GrB_Vector_free (&prob) ;
    GrB_Vector_free (&neighbor_max) ;
    GrB_Vector_free (&new_members) ;
    GrB_Vector_free (&new_neighbors) ;
    GrB_Vector_free (&candidates) ;
    GrB_Semiring_free (&maxSelect1st) ;
    GrB_BinaryOp_free (&set_random) ;
    GrB_Vector_free (&degrees) ;
    GrB_Vector_free (&Seed) ;
    GrB_Vector_free (&X) ;
    prand_finalize ( ) ;

    return (GrB_SUCCESS) ;
}

