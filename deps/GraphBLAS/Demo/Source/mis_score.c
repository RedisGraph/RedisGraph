//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/mis_score.c: set random score
//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

#include "demos.h"

//------------------------------------------------------------------------------
// mis_score: set the random score of a node
//------------------------------------------------------------------------------

// Assign a random number to each element scaled by the inverse of the node's
// degree.  This will increase the probability that low degree nodes are
// selected and larger sets are selected.

// this unary operator was used in V2.3.4, but it is not thread-safe
void mis_score (double *result, uint32_t *degree)
{
    // add 1 to prevent divide by zero
    double x = simple_rand_x ( ) ;
    (*result) = (0.0001 + x / (1. + 2.* (*degree))) ;
}

// a binary operator is thread-safe, where xrand is an entry from a
// vector of random numbers
void mis_score2 (double *result, uint32_t *degree, double *xrand)
{
    (*result) = (0.0001 + (*xrand) / (1. + 2.* (*degree))) ;
}

