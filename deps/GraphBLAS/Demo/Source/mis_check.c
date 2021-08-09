//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/mis_check.c: maximal independent set, w/error checking
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

// No copyright claim is made for this particular file; the above copyright
// applies to all of SuiteSparse:GraphBLAS, not this file.

// This method has been updated as of Version 2.2 of SuiteSparse:GraphBLAS.  It
// now uses GrB_vxm instead of GrB_mxv.

// This version also relies on predefined monoids and semirings, just to
// give an example of their use.

#include "GraphBLAS.h"

#ifdef GxB_SUITESPARSE_GRAPHBLAS
    // use predefined semirings.  They are safe to free,
    // so the FREE_ALL macro can be used as-is in either case.
    #define maxSelect1st    GxB_MAX_FIRST_FP64
#endif

// "OK(x)" macro defined in graphblas_demos.h calls a GraphBLAS method, and if
// it fails, prints the error, frees workspace, and returns to the caller.  It
// uses the FREE_ALL macro to free the workspace.
#define FREE_ALL                        \
    GrB_Vector_free (&iset) ;           \
    GrB_Vector_free (&prob) ;           \
    GrB_Vector_free (&neighbor_max) ;   \
    GrB_Vector_free (&new_members) ;    \
    GrB_Vector_free (&new_neighbors) ;  \
    GrB_Vector_free (&candidates) ;     \
    GrB_Semiring_free (&maxSelect1st) ; \
    GrB_Descriptor_free (&r_desc) ;     \
    GrB_Descriptor_free (&sr_desc) ;    \
    GrB_BinaryOp_free (&set_random) ;   \
    GrB_Vector_free (&degrees) ;        \
    GrB_Vector_free (&Seed) ;           \
    GrB_Vector_free (&X) ;              \
    prand_finalize ( ) ;

#undef GB_PUBLIC
#define GB_LIBRARY
#include "graphblas_demos.h"

//------------------------------------------------------------------------------
// mis_check: maximal independent set, with error checking
//------------------------------------------------------------------------------

// A variant of Luby's randomized algorithm [Luby 1985]. 

// Given a numeric n x n adjacency matrix A of an unweighted and undirected
// graph (where the value true represents an edge), compute a maximal set of
// independent nodes and return it in a boolean n-vector, 'iset' where
// set[i] == true implies node i is a member of the set (the iset vector
// should be uninitialized on input.)

// The graph cannot have any self edges, and it must be symmetric.  These
// conditions are not checked.  Self-edges will cause the method to stall.

// Singletons require special treatment.  Since they have no neighbors, their
// prob is never greater than the max of their neighbors, so they never get
// selected and cause the method to stall.  To avoid this case they are removed
// from the candidate set at the begining, and added to the iset.

GB_PUBLIC
GrB_Info mis_check              // compute a maximal independent set
(
    GrB_Vector *iset_output,    // iset(i) = true if i is in the set
    const GrB_Matrix A,         // symmetric Boolean matrix
    int64_t seed                // random number seed
)
{

    // these are set to NULL so that FREE_ALL can safely free all objects
    // at any time
    GrB_Vector iset = NULL ;
    GrB_Vector prob = NULL ;            // random probability for each node
    GrB_Vector neighbor_max = NULL ;    // value of max neighbor probability
    GrB_Vector new_members = NULL ;     // set of new members to iset
    GrB_Vector new_neighbors = NULL ;   // new neighbors to new iset members
    GrB_Vector candidates = NULL ;      // candidate members to iset
#ifndef GxB_SUITESPARSE_GRAPHBLAS
    GrB_Semiring maxSelect1st = NULL ;  // Max/Select1st "semiring"
#endif
    GrB_Descriptor r_desc = NULL ;
    GrB_Descriptor sr_desc = NULL ;
    GrB_BinaryOp set_random = NULL ;
    GrB_Vector degrees = NULL ;
    GrB_Vector Seed = NULL ;
    GrB_Vector X = NULL ;

    GrB_Index n ;
    GrB_Info info ;

    OK (GrB_Matrix_nrows (&n, A)) ;                 // n = # of nodes in graph

    OK (GrB_Vector_new (&prob, GrB_FP64, n)) ;
    OK (GrB_Vector_new (&neighbor_max, GrB_FP64, n)) ;
    OK (GrB_Vector_new (&new_members, GrB_BOOL, n)) ;
    OK (GrB_Vector_new (&new_neighbors, GrB_BOOL, n)) ;
    OK (GrB_Vector_new (&candidates, GrB_BOOL, n)) ;

    // Initialize independent set vector, bool
    OK (GrB_Vector_new (&iset, GrB_BOOL, n)) ;

#ifndef GxB_SUITESPARSE_GRAPHBLAS
    // create the maxSelect1st semiring
    OK (GrB_Semiring_new (&maxSelect1st, GrB_MAX_MONOID_FP64, GrB_FIRST_FP64)) ;
#endif

    // descriptor: C_replace
    OK (GrB_Descriptor_new (&r_desc)) ;
    OK (GrB_Descriptor_set (r_desc, GrB_OUTP, GrB_REPLACE)) ;

    // create the random number seeds
    OK (prand_init ( )) ;
    OK (prand_seed (&Seed, seed, n, 0)) ;
    OK (GrB_Vector_new (&X, GrB_FP64, n)) ;

    // descriptor: C_replace + structural complement of mask
    OK (GrB_Descriptor_new (&sr_desc)) ;
    OK (GrB_Descriptor_set (sr_desc, GrB_MASK, GrB_COMP)) ;
    OK (GrB_Descriptor_set (sr_desc, GrB_OUTP, GrB_REPLACE)) ;

    // create the mis_score binary operator
    OK (GrB_BinaryOp_new (&set_random, mis_score2,
        GrB_FP64, GrB_UINT32, GrB_FP64)) ;

    // compute the degree of each node
    OK (GrB_Vector_new (&degrees, GrB_FP64, n)) ;
    OK (GrB_Matrix_reduce_Monoid (degrees, NULL, NULL, GrB_PLUS_MONOID_FP64,
        A, NULL)) ;

    // singletons are not candidates; they are added to iset first instead
    // candidates[degree != 0] = 1
    OK (GrB_Vector_assign_BOOL (candidates, degrees, NULL, true,
        GrB_ALL, n, NULL)) ; 

    // add all singletons to iset
    // iset[degree == 0] = 1
    OK (GrB_Vector_assign_BOOL (iset, degrees, NULL, true,
        GrB_ALL, n, sr_desc)) ; 

    // Iterate while there are candidates to check.
    GrB_Index nvals ;
    OK (GrB_Vector_nvals (&nvals, candidates)) ;

    int64_t last_nvals = nvals ;        // just for error-checking

    while (nvals > 0)
    {
        // sparsify the random number seeds (just keep it for each candidate) 
        OK (GrB_Vector_assign (Seed, candidates, NULL, Seed,
            GrB_ALL, n, r_desc)) ;

        // compute a random probability scaled by inverse of degree
        OK (prand_xget (X, Seed)) ;
        OK (GrB_Vector_eWiseMult_BinaryOp (prob, candidates, NULL, set_random,
            degrees, X, r_desc)) ;

        // compute the max probability of all neighbors
        OK (GrB_vxm (neighbor_max, candidates, NULL, maxSelect1st,
            prob, A, r_desc)) ;

        // select node if its probability is > than all its active neighbors
        OK (GrB_Vector_eWiseAdd_BinaryOp (new_members, NULL, NULL, GrB_GT_FP64,
            prob, neighbor_max, NULL)) ;

        // add new members to independent set.
        OK (GrB_Vector_eWiseAdd_BinaryOp (iset, NULL, NULL, GrB_LOR, iset,
            new_members, NULL)) ;

        // remove new members from set of candidates c = c & !new
        OK (GrB_Vector_apply (candidates, new_members, NULL, GrB_IDENTITY_BOOL,
            candidates, sr_desc)) ;

        OK (GrB_Vector_nvals (&nvals, candidates)) ;
        if (nvals == 0) { break ; }                  // early exit condition

        // Neighbors of new members can also be removed from candidates
        OK (GrB_vxm (new_neighbors, candidates, NULL,
            GrB_LOR_LAND_SEMIRING_BOOL, new_members, A, NULL)) ;

        OK (GrB_Vector_apply (candidates, new_neighbors, NULL,
            GrB_IDENTITY_BOOL, candidates, sr_desc)) ;

        OK (GrB_Vector_nvals (&nvals, candidates)) ;

        // this will not occur, unless the input is corrupted somehow,
        // or if two candidates have exactly the same score
        if (last_nvals == nvals)
        {
            printf ("stall!\n") ;
            OK (GrB_INVALID_VALUE) ;
        }
        last_nvals = nvals ;
    }

    // drop explicit false values
    OK (GrB_Vector_apply (iset, iset, NULL, GrB_IDENTITY_BOOL, iset, r_desc)) ;

    // return result
    *iset_output = iset ;
    iset = NULL ;           // set to NULL so FREE_ALL doesn't free it

    // free workspace
    FREE_ALL ;
    return (GrB_SUCCESS) ;
}

