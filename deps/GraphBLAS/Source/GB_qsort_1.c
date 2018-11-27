//------------------------------------------------------------------------------
// GB_qsort_1: sort an n-by-1 list of integers
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This sort is not stable, but it is used in GraphBLAS only on lists with
// unique integers.  So it does not need to be stable.

#include "GB.h"

// returns true if a < b
#define GB_lt(A,a,B,b)                  \
(                                       \
    A ## _0 [a] < B ## _0 [b]           \
)

// argument list
#define GB_arg(A) A ##_0

// argument list
#define GB_args(type,A) type A ## _0 [ ]

// argument list, with offset
#define GB_arg_offset(A,x) A ##_0 + x

// sort a 1-by-n list
#define GB_K 1

// swap A [a] and A [b]
#define GB_swap(A,a,b)                                                      \
{                                                                           \
    int64_t t = A ## _0 [a] ; A ## _0 [a] = A ## _0 [b] ; A ## _0 [b] = t ; \
}

#define GB_partition GB_partition_1
#define GB_quicksort GB_quicksort_1

#include "GB_qsort_template.c"

void GB_qsort_1         // sort array A of size 1-by-n
(
    int64_t A_0 [ ],    // size-n array
    const int64_t n
)
{ 
    uint64_t seed = n ;
    GB_quicksort (GB_arg (A), n, &seed) ;
}

