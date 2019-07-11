//------------------------------------------------------------------------------
// GB_qsort_1b: sort a 2-by-n list, using A [0][ ] as the sort key
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This sort is not stable, but it is used in GraphBLAS only on lists with
// unique tuples (i,x).  So it does not need to be stable.  Just the first
// entry i in each tuple (i,x) is used as the sort key.  The second item x in
// is an arbitrary item of size xsize.

#include "GB_qsort.h"

// returns true if a < b
#define GB_lt(A,a,B,b)                  \
(                                       \
    A ## _0 [a] < B ## _0 [b]           \
)

// argument list
#define GB_arg(A) A ## _0, A ## _1, xsize

// argument list
#define GB_args(type,A) type A ## _0 [ ], GB_void A ## _1 [ ], size_t xsize

// argument list, with offset
#define GB_arg_offset(A,x) A ## _0 + x, A ## _1 + (x)*xsize, xsize

// entry entry has a single key
#define GB_K 1

// swap A [a] and A [b]
#define GB_swap(A,a,b)                                                        \
{                                                                             \
    int64_t t0 = A ## _0 [a] ; A ## _0 [a] = A ## _0 [b] ; A ## _0 [b] = t0 ; \
    GB_void t1 [xsize] ;                                                      \
    memcpy (t1, A ## _1 + (a)*xsize, xsize) ;                                 \
    memcpy (A ## _1 + (a)*xsize, A ## _1 + (b)*xsize, xsize) ;                \
    memcpy (A ## _1 + (b)*xsize, t1, xsize) ;                                 \
}

#define GB_partition GB_partition_1b
#define GB_quicksort GB_quicksort_1b
#define GB_quicksort_par  GB_quicksort_par_1b
#define GB_quicksort_main GB_quicksort_main_1b

#include "GB_qsort_template.c"

void GB_qsort_1b        // sort array A of size 2-by-n, using 1 key (A [0][])
(
    int64_t A_0 [ ],    // size n array
    GB_void A_1 [ ],    // size n array
    const size_t xsize, // size of entries in A_1
    const int64_t n,
    GB_Context Context  // for # of threads; use one thread if NULL
)
{ 
    uint64_t seed = n ;
    GB_quicksort_main (GB_arg (A), n, &seed, Context) ;
}

