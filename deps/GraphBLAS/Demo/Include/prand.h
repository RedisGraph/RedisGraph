//------------------------------------------------------------------------------
// GraphBLAS/Demo/Include/prand.h: parallel random number generator
//------------------------------------------------------------------------------

// A simple parallel pseudo-random number generator.

// prand_init ( ) must be called before creating any random numbers or seeds.
// It creates the internal random seed type, and the operators to work on it.

// prand_finalize ( ) must be called when the application is finished with
// the prand functions.  It frees everything created by prand_init, but it
// does not free any Seed vectors.

// prand_seed (&Seed, seed, n, nthreads): creates a GrB_Vector Seed of n random
// number seeds, based on the scalar integer seed.  nthreads are used to
// construct the seed; if zero, the OpenMP default is used.  Free the vector
// with GrB_Vector_free (&Seed) when done.  The Seed vector is fully dense; all
// entries are present.

// prand_iget (X, Seed): fills an existing GrB_Vector X with n random integers
// (of type GrB_UINT64), and advances the Seed vector to its next state.  The
// vector X and Seed must have the same length.  If X is not GrB_UINT64, then
// the values are typecasted from GrB_UINT64 into whatever type X is.  If the
// Seed is sparse, the X vector will have the same nonzero pattern as the Seed
// vector.

// prand_xget (X, Seed): the same as prand_iget, except that X has type
// type GrB_FP64, and the values are in the range 0 to 1 inclusive.

// prand_print (Seed, pr): prints the Seed vector.  pr = 0: prints nothing,
// 1: prints a few entries, 2: prints everything.  This is not needed for
// production use, just for testing.

// Each function returns its status as a GrB_Info value.  Note that a Seed
// vector can be used in either prand_iget or prand_xget, or any combination.

/* Example usage:

        GrB_Vector Seed, X, Y ;
        GrB_Index n = 10 ;
        prand_init ( ) ;                // create the prand types and operators

        prand_seed (&Seed, 42, n, 0) ;  // create a vector of 10 seeds
        GrB_Vector_new (&X, GrB_FP64, n) ;
        GrB_Vector_new (&Y, GrB_UINT64, n) ;

        prand_seed (&Another, 99, 2*n, 0) ;  // create a vector of 20 seeds
        GrB_Vector_new (&Z, GrB_UINT64, 2*n) ;

        for (int trial = 0 ; trial < 1000 ; trial++)
        {
            prand_xget (X, Seed) ;      // fill X with n random doubles
            prand_iget (Y, Seed) ;      // fill Y with n random uint64's
            prand_iget (Z, Another) ;   // fill Z with 2*n random uint64's
        }

        GrB_Vector_free (&Seed) ;              // free the vectors
        GrB_Vector_free (&Another) ;
        GrB_Vector_free (&X) ;
        GrB_Vector_free (&Y) ;

        prand_finalize ( ) ;            // free the prand types and operators
*/

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef PRAND_H
#define PRAND_H

#include "GraphBLAS.h"

// prand_init:  create the random seed type and its operators
GB_PUBLIC
GrB_Info prand_init ( ) ;

// prand_finalize:  free the random seed type and its operators
GB_PUBLIC
GrB_Info prand_finalize ( ) ;

// prand_seed:  create a Seed vector of random number seeds
GB_PUBLIC
GrB_Info prand_seed
(
    GrB_Vector *Seed,   // vector of random number seeds
    int64_t seed,       // scalar input seed
    GrB_Index n,        // size of Seed to create
    int nthreads        // # of threads to use (OpenMP default if <= 0)
) ;

// prand_iget: return a vector of random uint64 integers
GB_PUBLIC
GrB_Info prand_iget
(
    GrB_Vector X,
    GrB_Vector Seed
) ;

// prand_xget: return a vector of random doubles, in range 0 to 1 inclusive
GB_PUBLIC
GrB_Info prand_xget
(
    GrB_Vector X,
    GrB_Vector Seed
) ;

// prand_print:  print the Seed vector
GB_PUBLIC
GrB_Info prand_print
(
    GrB_Vector Seed,
    int pr              // 0: print nothing, 1: print some, 2: print all
) ;

// prand_next: advance the seed
GB_PUBLIC
GrB_Info prand_next
(
    GrB_Vector Seed
) ;

//------------------------------------------------------------------------------
// prand_t: the random number seed
//------------------------------------------------------------------------------

typedef struct
{
    uint64_t seed [5] ;      // random seed
}
prand_t ;

GB_PUBLIC
void prand_next_f (prand_t *z, const prand_t *x) ;
GB_PUBLIC
void prand_iget_f (uint64_t *z, const prand_t *x) ;
GB_PUBLIC
void prand_xget_f (double *z, prand_t *x) ;
GB_PUBLIC
void prand_dup_f (prand_t *z, const prand_t *x, const prand_t *y) ;

#endif

