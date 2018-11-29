//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/mis_check.c: maximal independent set, w/error checking
//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

// This method has been updated as of Version 2.2 of SuiteSparse:GraphBLAS.  It
// now uses GrB_vxm instead of GrB_mxv.

// This version also relies on predefined monoids and semirings, just to
// give an example of their use.

#include "GraphBLAS.h"

#ifdef GxB_SUITESPARSE_GRAPHBLAS
    // use predefined semirings.  They are safe to free,
    // so the FREE_ALL macro can be used as-is in either case.
    #define Max             GxB_MAX_FP32_MONOID
    #define maxSelect1st    GxB_MAX_FIRST_FP32
    #define Lor             GxB_LOR_BOOL_MONOID
    #define Boolean         GxB_LOR_LAND_BOOL
#endif

// "OK(x)" macro defined in demos.h calls a GraphBLAS method, and if it fails,
// prints the error, frees workspace, and returns to the caller.  It uses the
// FREE_ALL macro to free the workspace.
#define FREE_ALL                \
    GrB_free (&iset) ;          \
    GrB_free (&prob) ;          \
    GrB_free (&neighbor_max) ;  \
    GrB_free (&new_members) ;   \
    GrB_free (&new_neighbors) ; \
    GrB_free (&candidates) ;    \
    GrB_free (&Max) ;           \
    GrB_free (&maxSelect1st) ;  \
    GrB_free (&Lor) ;           \
    GrB_free (&Boolean) ;       \
    GrB_free (&r_desc) ;        \
    GrB_free (&sr_desc) ;       \
    GrB_free (&set_random) ;    \
    GrB_free (&degrees) ;

#include "demos.h"

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

GrB_Info mis_check              // compute a maximal independent set
(
    GrB_Vector *iset_output,    // iset(i) = true if i is in the set
    const GrB_Matrix A          // symmetric Boolean matrix
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
    GrB_Monoid Max = NULL ;
    GrB_Semiring maxSelect1st = NULL ;  // Max/Select1st "semiring"
    GrB_Monoid Lor = NULL ;
    GrB_Semiring Boolean = NULL ;       // Boolean semiring
#endif
    GrB_Descriptor r_desc = NULL ;
    GrB_Descriptor sr_desc = NULL ;
    GrB_UnaryOp set_random = NULL ;
    GrB_Vector degrees = NULL ;

    GrB_Index n ;
    GrB_Info info ;

    OK (GrB_Matrix_nrows (&n, A)) ;                 // n = # of nodes in graph

    OK (GrB_Vector_new (&prob, GrB_FP32, n)) ;
    OK (GrB_Vector_new (&neighbor_max, GrB_FP32, n)) ;
    OK (GrB_Vector_new (&new_members, GrB_BOOL, n)) ;
    OK (GrB_Vector_new (&new_neighbors, GrB_BOOL, n)) ;
    OK (GrB_Vector_new (&candidates, GrB_BOOL, n)) ;

    // Initialize independent set vector, bool
    OK (GrB_Vector_new (&iset, GrB_BOOL, n)) ;

#ifndef GxB_SUITESPARSE_GRAPHBLAS
    // create the maxSelect1st semiring
    OK (GrB_Monoid_new (&Max, GrB_MAX_FP32, (float) 0.0)) ;
    OK (GrB_Semiring_new (&maxSelect1st, Max, GrB_FIRST_FP32)) ;

    // create the OR-AND-BOOL semiring
    OK (GrB_Monoid_new (&Lor, GrB_LOR, (bool) false)) ;
    OK (GrB_Semiring_new (&Boolean, Lor, GrB_LAND)) ;
#endif

    // descriptor: C_replace
    OK (GrB_Descriptor_new (&r_desc)) ;
    OK (GrB_Descriptor_set (r_desc, GrB_OUTP, GrB_REPLACE)) ;

    // descriptor: C_replace + structural complement of mask
    OK (GrB_Descriptor_new (&sr_desc)) ;
    OK (GrB_Descriptor_set (sr_desc, GrB_MASK, GrB_SCMP)) ;
    OK (GrB_Descriptor_set (sr_desc, GrB_OUTP, GrB_REPLACE)) ;

    // create the mis_score unary operator
    OK (GrB_UnaryOp_new (&set_random, mis_score, GrB_FP32, GrB_UINT32)) ;

    // compute the degree of each node
    OK (GrB_Vector_new (&degrees, GrB_FP64, n)) ;
    OK (GrB_reduce (degrees, NULL, NULL, GrB_PLUS_FP64, A, NULL)) ;

    // singletons are not candidates; they are added to iset first instead
    // candidates[degree != 0] = 1
    OK (GrB_assign (candidates, degrees, NULL, true, GrB_ALL, n, NULL)) ; 

    // add all singletons to iset
    // iset[degree == 0] = 1
    OK (GrB_assign (iset, degrees, NULL, true, GrB_ALL, n, sr_desc)) ; 

    // Iterate while there are candidates to check.
    GrB_Index nvals ;
    OK (GrB_Vector_nvals (&nvals, candidates)) ;

    int64_t last_nvals = nvals ;        // just for error-checking

    while (nvals > 0)
    {

        // compute a random probability scaled by inverse of degree
        OK (GrB_apply (prob, candidates, NULL, set_random, degrees, r_desc)) ;

        // compute the max probability of all neighbors
        OK (GrB_vxm (neighbor_max, candidates, NULL, maxSelect1st,
            prob, A, r_desc)) ;

        // select node if its probability is > than all its active neighbors
        OK (GrB_eWiseAdd (new_members, NULL, NULL, GrB_GT_FP64, prob,
            neighbor_max, NULL)) ;

        // add new members to independent set.
        OK (GrB_eWiseAdd (iset, NULL, NULL, GrB_LOR, iset, new_members, NULL)) ;

        // remove new members from set of candidates c = c & !new
        OK (GrB_apply (candidates, new_members, NULL, GrB_IDENTITY_BOOL,
            candidates, sr_desc)) ;

        OK (GrB_Vector_nvals (&nvals, candidates)) ;
        if (nvals == 0) { break ; }                  // early exit condition

        // Neighbors of new members can also be removed from candidates
        OK (GrB_vxm (new_neighbors, candidates, NULL, Boolean,
            new_members, A, NULL)) ;
        OK (GrB_apply (candidates, new_neighbors, NULL, GrB_IDENTITY_BOOL,
            candidates, sr_desc)) ;

        OK (GrB_Vector_nvals (&nvals, candidates)) ;

        // this will not occur, unless the input is corrupted somehow
        if (last_nvals == nvals) { printf ("stall!\n") ; OK (GrB_INVALID_VALUE) ; }
        last_nvals = nvals ;
    }

    // drop explicit false values
    OK (GrB_apply (iset, iset, NULL, GrB_IDENTITY_BOOL, iset, r_desc)) ;

    // return result
    *iset_output = iset ;
    iset = NULL ;           // set to NULL so FREE_ALL doesn't free it

    // free workspace
    FREE_ALL ;
    return (GrB_SUCCESS) ;
}

