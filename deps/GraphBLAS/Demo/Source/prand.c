//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/prand: parallel random number generator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// A simple thread-safe parallel pseudo-random nuumber generator.

#include "GraphBLAS.h"
#undef GB_PUBLIC
#define GB_LIBRARY
#include "graphblas_demos.h"

//------------------------------------------------------------------------------
// prand macros
//------------------------------------------------------------------------------

// Generate the next seed, and extract a random 15-bit value from a seed.

#define PRAND_RECURENCE(seed) ((seed) * 1103515245 + 12345)

#define PRAND_15_MAX 32767 
#define PRAND_15(seed) (((seed)/65536) % (PRAND_15_MAX + 1))

//------------------------------------------------------------------------------
// global types and operators
//------------------------------------------------------------------------------

// These can be shared by all threads in a user application, and thus are
// safely declared as global objects.

GrB_Type prand_type = NULL ;
GrB_UnaryOp prand_next_op = NULL ;
GrB_UnaryOp prand_iget_op = NULL ;
GrB_UnaryOp prand_xget_op = NULL ;
GrB_BinaryOp prand_dup_op = NULL ;

//------------------------------------------------------------------------------
// prand_next_op:  unary operator to construct the next seed
//------------------------------------------------------------------------------

// z = f(x), where x is the old seed and z is the new seed.

GB_PUBLIC
void prand_next_f (prand_t *z, const prand_t *x)
{
    for (int k = 0 ; k < 5 ; k++)
    {
        z->seed [k] = PRAND_RECURENCE (x->seed [k]) ;
    }
}

//------------------------------------------------------------------------------
// prand_iget:  unary operator to construct get a random integer from the seed
//------------------------------------------------------------------------------

// z = f(x), where x is a random seed, and z is an unsigned 64-bit
// pseudo-random number constructed from the seed.

GB_PUBLIC
void prand_iget_f (uint64_t *z, const prand_t *x)
{
    uint64_t i = 0 ;
    for (int k = 0 ; k < 5 ; k++)
    {
        i = PRAND_15_MAX * i + PRAND_15 (x->seed [k]) ;
    }
    (*z) = i ;
}

//------------------------------------------------------------------------------
// prand_xget:  unary operator to construct get a random double from the seed
//------------------------------------------------------------------------------

// z = f(x), where x is a random seed, and z is a double precision
// pseudo-random number constructed from the seed, in the range 0 to 1.

GB_PUBLIC
void prand_xget_f (double *z, prand_t *x)
{
    uint64_t i ;
    prand_iget_f (&i, x) ;
    (*z) = ((double) i) / ((double) UINT64_MAX) ;
}

//------------------------------------------------------------------------------
// prand_dup:  binary operator to build a vector
//------------------------------------------------------------------------------

// This is required by GrB_Vector_build, but is never called since no
// duplicates are created.  This is the SECOND operator for the prand_type.

#if defined ( __INTEL_COMPILER )
// disable icc warnings
//  869:  unused parameters
#pragma warning (disable: 869 )
#elif defined (  __GNUC__ )
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

GB_PUBLIC
void prand_dup_f (prand_t *z, /* unused: */ const prand_t *x, const prand_t *y)
{
    (*z) = (*y) ;
}

//------------------------------------------------------------------------------
// prand_init:  create the random seed type and its operators
//------------------------------------------------------------------------------

#define PRAND_FREE_ALL                                      \
{                                                           \
    GrB_Type_free (&prand_type) ;                                \
    GrB_UnaryOp_free (&prand_next_op) ;                             \
    GrB_UnaryOp_free (&prand_iget_op) ;                             \
    GrB_UnaryOp_free (&prand_xget_op) ;                             \
    GrB_BinaryOp_free (&prand_dup_op) ;                              \
}

#undef  OK
#define OK(method)                                          \
{                                                           \
    GrB_Info info = method ;                                \
    if (info != GrB_SUCCESS)                                \
    {                                                       \
        PRAND_FREE_ALL ;                                    \
        printf ("GraphBLAS error: %d\n", info) ;            \
        return (info) ;                                     \
    }                                                       \
}

GB_PUBLIC
GrB_Info prand_init ( )
{
    prand_type = NULL ;
    prand_next_op = NULL ;
    prand_iget_op = NULL ;
    prand_xget_op = NULL ;
    prand_dup_op = NULL ;
    OK (GrB_Type_new (&prand_type, sizeof (prand_t))) ;
    OK (GrB_UnaryOp_new (&prand_next_op, (GxB_unary_function) prand_next_f,
        prand_type, prand_type)) ;
    OK (GrB_UnaryOp_new (&prand_iget_op, (GxB_unary_function) prand_iget_f,
        GrB_UINT64, prand_type)) ;
    OK (GrB_UnaryOp_new (&prand_xget_op, (GxB_unary_function) prand_xget_f,
        GrB_FP64, prand_type)) ;
    OK (GrB_BinaryOp_new (&prand_dup_op, (GxB_binary_function) prand_dup_f,
        prand_type, prand_type, prand_type)) ;
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// prand_finalize:  free the random seed type and its operators
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info prand_finalize ( )
{
    PRAND_FREE_ALL ;
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// prand_next: get the next random numbers
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info prand_next
(
    GrB_Vector Seed
)
{
    return (GrB_Vector_apply (Seed, NULL, NULL, prand_next_op, Seed, NULL)) ;
}

//------------------------------------------------------------------------------
// prand_seed:  create a vector of random seeds
//------------------------------------------------------------------------------

// Returns a vector of random seed values.

#define PRAND_FREE_WORK                                     \
{                                                           \
    free (I) ;                                              \
    free (X) ;                                              \
}

#undef  PRAND_FREE_ALL
#define PRAND_FREE_ALL                                      \
{                                                           \
    PRAND_FREE_WORK ;                                       \
    GrB_Vector_free (Seed) ;                                \
}

GB_PUBLIC
GrB_Info prand_seed
(
    GrB_Vector *Seed,   // vector of random number seeds
    int64_t seed,       // scalar input seed
    GrB_Index n,        // size of Seed to create
    int nthreads        // # of threads to use (OpenMP default if <= 0)
)
{

    GrB_Index *I = NULL ;
    prand_t   *X = NULL ;

    // allocate the Seed vector
    OK (GrB_Vector_new (Seed, prand_type, n)) ;

    // allocate the I and X arrays
    I = (GrB_Index *) malloc ((n+1) * sizeof (GrB_Index)) ;
    X = (prand_t *) malloc ((n+1) * sizeof (prand_t)) ;
    if (I == NULL || X == NULL)
    {
        PRAND_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    // determine # of threads to use
    int nthreads_max = 1 ;
    #ifdef _OPENMP
    nthreads_max = omp_get_max_threads ( ) ;
    #endif
    if (nthreads <= 0 || nthreads > nthreads_max)
    {
        nthreads = nthreads_max ;
    }

    // construct the tuples for the initial seeds
    int64_t i, len = (int64_t) n  ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (i = 0 ; i < len ; i++)
    {
        I [i] = i ;
        for (int k = 0 ; k < 5 ; k++)
        {
            X [i].seed [k] = (100000000*(seed) + 10*i + k + 1) ;
        }
    }

    // build the Seed vector
    OK (GrB_Vector_build_UDT (*Seed, I, X, n, prand_dup_op)) ;

    // free workspace
    PRAND_FREE_WORK ;

    // advance to the first set of random numbers
    OK (prand_next (*Seed)) ;

    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// prand_print:  print the Seed vector
//------------------------------------------------------------------------------

// This is meant for testing, not production use.

#undef  PRAND_FREE_ALL
#define PRAND_FREE_ALL ;

GB_PUBLIC
GrB_Info prand_print
(
    GrB_Vector Seed,
    int pr              // 0: print nothing, 1: print some, 2: print all
)
{
    if (pr > 0)
    {
        GrB_Index n ;
        OK (GrB_Vector_nvals (&n, Seed)) ;
        printf ("\nSeed: length %g\n", (double) n) ;
        prand_t x ;
        for (int k = 0 ; k < 5 ; k++) x.seed [k] = -1 ;
        for (int64_t i = 0 ; i < (int64_t) n ; i++)
        {
            if (GrB_Vector_extractElement_UDT (&x, Seed, i) == GrB_SUCCESS)
            {
                printf ("%g: ", (double) i) ;
                for (int k = 0 ; k < 5 ; k++)
                {
                    printf (" %.18g", (double) (x.seed [k])) ;
                }
                printf ("\n") ;
            }
            if (pr == 1 && i > 10) break ;
        }
    }
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// prand_iget: return a vector of random uint64 integers
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info prand_iget
(
    GrB_Vector X,
    GrB_Vector Seed
)
{
    OK (GrB_Vector_apply (X, NULL, NULL, prand_iget_op, Seed, NULL)) ;
    return (prand_next (Seed)) ;
}

//------------------------------------------------------------------------------
// prand_xget: return a vector of random doubles, in range 0 to 1 inclusive
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info prand_xget
(
    GrB_Vector X,
    GrB_Vector Seed
)
{
    OK (GrB_Vector_apply (X, NULL, NULL, prand_xget_op, Seed, NULL)) ;
    return (prand_next (Seed)) ;
}

