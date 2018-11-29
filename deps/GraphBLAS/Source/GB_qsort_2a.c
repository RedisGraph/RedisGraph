//------------------------------------------------------------------------------
// GB_qsort_2a: sort a 2-by-n list of integers, using A [0][ ] as the sort key
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This sort is not stable, but it is used in GraphBLAS only on lists with
// unique tuples (i,k).  So it does not need to be stable.  Just the first
// entry i in each tuple (i,k) is used as the sort key.  The second item k in
// each tuple happens to be unique in itself, but this is not part of the
// sort key.

#include "GB.h"

// returns true if a < b
#define GB_lt(A,a,B,b)                  \
(                                       \
    A ## _0 [a] < B ## _0 [b]           \
)

// argument list
#define GB_arg(A) A ## _0, A ## _1

// argument list
#define GB_args(type,A) type A ## _0 [ ], type A ## _1 [ ]

// argument list, with offset
#define GB_arg_offset(A,x) A ## _0 + x, A ## _1 + x

// sort a 2-by-n list
#define GB_K 2

// swap A [a] and A [b]
#define GB_swap(A,a,b)                                                        \
{                                                                             \
    int64_t t0 = A ## _0 [a] ; A ## _0 [a] = A ## _0 [b] ; A ## _0 [b] = t0 ; \
    int64_t t1 = A ## _1 [a] ; A ## _1 [a] = A ## _1 [b] ; A ## _1 [b] = t1 ; \
}

#define GB_partition GB_partition_2a
#define GB_quicksort GB_quicksort_2a

#include "GB_qsort_template.c"

void GB_qsort_2a        // sort array A of size 2-by-n, using 1 key (A [0][])
(
    int64_t A_0 [ ],      // size n array
    int64_t A_1 [ ],      // size n array
    int64_t n
)
{ 
    uint64_t seed = n ;
    GB_quicksort (GB_arg (A), n, &seed) ;
}

