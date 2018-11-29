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

void mis_score (float *result, uint32_t *degree)
{
    // add 1 to prevent divide by zero
    float x = simple_rand_x ( ) ;
    (*result) = (0.0001f + x / (1. + 2.* (*degree))) ;
}

