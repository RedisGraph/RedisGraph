//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/mis.c: maximal independent set
//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

// This method has been updated as of Version 2.2 of SuiteSparse:GraphBLAS.  It
// now uses GrB_vxm instead of GrB_mxv.

#include "demos.h"

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
// conditions are not checked.  Self-edges will cause the method to stall.

// Singletons require special treatment.  Since they have no neighbors, their
// prob is never greater than the max of their neighbors, so they never get
// selected and cause the method to stall.  To avoid this case they are removed
// from the candidate set at the begining, and added to the iset.

GrB_Info mis                    // compute a maximal independent set
(
    GrB_Vector *iset_output,    // iset(i) = true if i is in the set
    const GrB_Matrix A          // symmetric Boolean matrix
)
{

    GrB_Vector iset = NULL ;
    GrB_Vector prob = NULL ;            // random probability for each node
    GrB_Vector neighbor_max = NULL ;    // value of max neighbor probability
    GrB_Vector new_members = NULL ;     // set of new members to iset
    GrB_Vector new_neighbors = NULL ;   // new neighbors to new iset members
    GrB_Vector candidates = NULL ;      // candidate members to iset
    GrB_Monoid Max = NULL ;
    GrB_Semiring maxSelect1st = NULL ;  // Max/Select1st "semiring"
    GrB_Monoid Lor = NULL ;
    GrB_Semiring Boolean = NULL ;       // Boolean semiring
    GrB_Descriptor r_desc = NULL ;
    GrB_Descriptor sr_desc = NULL ;
    GrB_UnaryOp set_random = NULL ;
    GrB_Vector degrees = NULL ;

    GrB_Index n ;

    GrB_Matrix_nrows (&n, A) ;                 // n = # of nodes in graph

    GrB_Vector_new (&prob, GrB_FP32, n) ;
    GrB_Vector_new (&neighbor_max, GrB_FP32, n) ;
    GrB_Vector_new (&new_members, GrB_BOOL, n) ;
    GrB_Vector_new (&new_neighbors, GrB_BOOL, n) ;
    GrB_Vector_new (&candidates, GrB_BOOL, n) ;

    // Initialize independent set vector, bool
    GrB_Vector_new (&iset, GrB_BOOL, n) ;

    // create the maxSelect1st semiring
    GrB_Monoid_new (&Max, GrB_MAX_FP32, (float) 0.0) ;
    GrB_Semiring_new (&maxSelect1st, Max, GrB_FIRST_FP32) ;

    // create the OR-AND-BOOL semiring
    GrB_Monoid_new (&Lor, GrB_LOR, (bool) false) ;
    GrB_Semiring_new (&Boolean, Lor, GrB_LAND) ;

    // descriptor: C_replace
    GrB_Descriptor_new (&r_desc) ;
    GrB_Descriptor_set (r_desc, GrB_OUTP, GrB_REPLACE) ;

    // descriptor: C_replace + structural complement of mask
    GrB_Descriptor_new (&sr_desc) ;
    GrB_Descriptor_set (sr_desc, GrB_MASK, GrB_SCMP) ;
    GrB_Descriptor_set (sr_desc, GrB_OUTP, GrB_REPLACE) ;

    // create the mis_score unary operator
    GrB_UnaryOp_new (&set_random, mis_score, GrB_FP32, GrB_UINT32) ;

    // compute the degree of each nodes
    GrB_Vector_new (&degrees, GrB_FP64, n) ;
    GrB_reduce (degrees, NULL, NULL, GrB_PLUS_FP64, A, NULL) ;

    // singletons are not candidates; they are added to iset first instead
    // candidates[degree != 0] = 1
    GrB_assign (candidates, degrees, NULL, true, GrB_ALL, n, NULL) ; 

    // add all singletons to iset
    // iset[degree == 0] = 1
    GrB_assign (iset, degrees, NULL, true, GrB_ALL, n, sr_desc) ; 

    // Iterate while there are candidates to check.
    GrB_Index nvals ;
    GrB_Vector_nvals (&nvals, candidates) ;

    int64_t last_nvals = nvals ;

    while (nvals > 0)
    {
        // compute a random probability scaled by inverse of degree
        GrB_apply (prob, candidates, NULL, set_random, degrees, r_desc) ;

        // compute the max probability of all neighbors
        GrB_vxm (neighbor_max, candidates, NULL, maxSelect1st,
            prob, A, r_desc) ;

        // select node if its probability is > than all its active neighbors
        GrB_eWiseAdd (new_members, NULL, NULL, GrB_GT_FP64, prob,
            neighbor_max, NULL) ;

        // add new members to independent set.
        GrB_eWiseAdd (iset, NULL, NULL, GrB_LOR, iset, new_members, NULL) ;

        // remove new members from set of candidates c = c & !new
        GrB_apply (candidates, new_members, NULL, GrB_IDENTITY_BOOL,
            candidates, sr_desc) ;

        GrB_Vector_nvals (&nvals, candidates) ;
        if (nvals == 0) { break ; }                  // early exit condition

        // Neighbors of new members can also be removed from candidates
        GrB_vxm (new_neighbors, candidates, NULL, Boolean,
            new_members, A, NULL) ;
        GrB_apply (candidates, new_neighbors, NULL, GrB_IDENTITY_BOOL,
            candidates, sr_desc) ;

        GrB_Vector_nvals (&nvals, candidates) ;

        // this will not occur, unless the input is corrupted somehow
        if (last_nvals == nvals) { printf ("stall!\n") ; exit (1) ; }
        last_nvals = nvals ;
    }

    // drop explicit false values
    GrB_apply (iset, iset, NULL, GrB_IDENTITY_BOOL, iset, r_desc) ;

    // return result
    *iset_output = iset ;

    // free workspace
    GrB_free (&prob) ;
    GrB_free (&neighbor_max) ;
    GrB_free (&new_members) ;
    GrB_free (&new_neighbors) ;
    GrB_free (&candidates) ;
    GrB_free (&Max) ;
    GrB_free (&maxSelect1st) ;
    GrB_free (&Lor) ;
    GrB_free (&Boolean) ;
    GrB_free (&r_desc) ;
    GrB_free (&sr_desc) ;
    GrB_free (&set_random) ;
    GrB_free (&degrees) ;

    return (GrB_SUCCESS) ;
}

