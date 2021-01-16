//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/mis_score.c: set random score
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

// No copyright claim is made for this particular file; the above copyright
// applies to all of SuiteSparse:GraphBLAS, not this file.

#include "GraphBLAS.h"
#undef GB_PUBLIC
#define GB_LIBRARY
#include "graphblas_demos.h"

//------------------------------------------------------------------------------
// mis_score: set the random score of a node
//------------------------------------------------------------------------------

// Assign a random number to each element scaled by the inverse of the node's
// degree.  This will increase the probability that low degree nodes are
// selected and larger sets are selected.

// this unary operator was used in V2.3.4, but it is not thread-safe
GB_PUBLIC
void mis_score (void *result, const void *degree)
{
    // add 1 to prevent divide by zero
    uint32_t deg = (*((uint32_t *) degree)) ;
    double x = simple_rand_x ( ) ;
    (*((double *) result)) = (0.0001 + x / (1. + 2.* deg)) ;
}

// a binary operator is thread-safe, where xrand is an entry from a
// vector of random numbers
GB_PUBLIC
void mis_score2 (void *result, const void *degree, const void *xrand)
{
    uint32_t deg = (*((uint32_t *) degree)) ;
    double x = (*((double *) xrand)) ;
    (*((double *) result)) = (0.0001 + x / (1. + 2.* deg)) ;
}

