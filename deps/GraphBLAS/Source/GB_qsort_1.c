//------------------------------------------------------------------------------
// GB_qsort_1: sort an n-by-1 list of integers
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This sort is not stable, but it is used in GraphBLAS only on lists with
// unique integers.  So it does not need to be stable.

#include "GB.h"

// returns true if a < b
#define lt(A,a,B,b)                     \
(                                       \
    A ## _0 [a] < B ## _0 [b]           \
)

// argument list
#define arg(A) A ##_0

// argument list
#define args(type,A) type A ## _0 [ ]

// argument list, with offset
#define arg_offset(A,x) A ##_0 + x

// sort a 1-by-n list
#define K 1

// swap A [a] and A [b]
#define swap(A,a,b)                                                         \
{                                                                           \
    int64_t t = A ## _0 [a] ; A ## _0 [a] = A ## _0 [b] ; A ## _0 [b] = t ; \
}

#define partition partition_1
#define quicksort quicksort_1

#include "GB_qsort_template.c"

void GB_qsort_1         // sort array A of size 1-by-n
(
    int64_t A_0 [ ],    // size-n array
    const int64_t n
)
{
    quicksort (arg (A), n) ;
}

#undef K
#undef lt
#undef arg
#undef args
#undef arg_offset
#undef swap
#undef partition
#undef quicksort

