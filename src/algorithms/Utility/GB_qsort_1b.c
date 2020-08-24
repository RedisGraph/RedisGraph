//------------------------------------------------------------------------------
// GB_qsort_1b: sort a 2-by-n list, using A [0][ ] as the sort key
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_sort.h"

// returns true if A [a] < B [b]
#define GB_lt(A,a,B,b)                  \
    GB_lt_1 (A ## _0, a, B ## _0, b)

// argument list for calling a function
#define GB_arg(A)                       \
    A ## _0, A ## _1, xsize

// argument list for calling a function, with offset
#define GB_arg_offset(A,x)              \
    A ## _0 + (x), A ## _1 + (x)*xsize, xsize

// argument list for defining a function
#define GB_args(A)                      \
    int64_t *LA_RESTRICT A ## _0,       \
    GB_void *LA_RESTRICT A ## _1,       \
    size_t xsize

// each entry has a single key
#define GB_K 1

// swap A [a] and A [b]
#define GB_swap(A,a,b)                                                      \
{                                                                           \
    int64_t t = A ## _0 [a] ; A ## _0 [a] = A ## _0 [b] ; A ## _0 [b] = t ; \
    GB_void t1 [xsize] ;                                                    \
    memcpy (t1, A ## _1 + (a)*xsize, xsize) ;                               \
    memcpy (A ## _1 + (a)*xsize, A ## _1 + (b)*xsize, xsize) ;              \
    memcpy (A ## _1 + (b)*xsize, t1, xsize) ;                               \
}

#define GB_partition GB_partition_1b
#define GB_quicksort GB_quicksort_1b

#include "GB_qsort_template.i"

void GB_qsort_1b    // sort array A of size 2-by-n, using 1 key (A [0][])
(
    int64_t *LA_RESTRICT A_0,   // size n array
    GB_void *LA_RESTRICT A_1,   // size n array
    const size_t xsize,         // size of entries in A_1
    const int64_t n
)
{ 
    uint64_t seed = n ;
    GB_quicksort (GB_arg (A), n, &seed) ;
}

