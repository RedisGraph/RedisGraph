//------------------------------------------------------------------------------
// GB_qsort_3: sort a 3-by-n list of integers, using A[0:2][] as the key
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This sort is not stable, but it is used in GraphBLAS only on lists with
// unique tuples (j,i,k).  So it does not need to be stable.  All entries j, i
// and k in the tuples (j,i,k) are used as the sort key.  The values i and j
// may appear in multiple tuples, but the value k is unique across all tuples.

#include "GB.h"

// returns true if a < b
#define GB_lt(A,a,B,b)                                                      \
(                                                                           \
    /* a and b are tuples of the form (j,i,k) where j is a column index, */ \
    /* i is a row index and */                                              \
    /* k is the original position in the input array of that entry. */      \
    /* If a and b have the same indices, compare their positions, k. */     \
    (A ## _0 [a] < B ## _0 [b]) ?                                           \
    (                                                                       \
        true                                                                \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        (A ## _0 [a] == B ## _0 [b]) ?                                      \
        (                                                                   \
            /* col indices are the same; check row indices */               \
            (A ## _1 [a] < B ## _1 [b]) ?                                   \
            (                                                               \
                true                                                        \
            )                                                               \
            :                                                               \
            (                                                               \
                (A ## _1 [a] == B ## _1 [b]) ?                              \
                (                                                           \
                    /* indices are the same; check the 3rd entry, k, */     \
                    /* which is always unique */                            \
                    (A ## _2 [a] < B ## _2 [b])                             \
                )                                                           \
                :                                                           \
                (                                                           \
                    false                                                   \
                )                                                           \
            )                                                               \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            false                                                           \
        )                                                                   \
    )                                                                       \
)

// argument list
#define GB_arg(A) A ## _0, A ## _1, A ## _2

// argument list
#define GB_args(type,A) type A ## _0 [ ], type A ## _1 [ ], type A ## _2 [ ]

// argument list, with offset
#define GB_arg_offset(A,x) A ## _0 + x, A ## _1 + x, A ## _2 + x

// sort a 3-by-n list
#define GB_K 3

// swap A [a] and A [b]
#define GB_swap(A,a,b)                                                        \
{                                                                             \
    int64_t t0 = A ## _0 [a] ; A ## _0 [a] = A ## _0 [b] ; A ## _0 [b] = t0 ; \
    int64_t t1 = A ## _1 [a] ; A ## _1 [a] = A ## _1 [b] ; A ## _1 [b] = t1 ; \
    int64_t t2 = A ## _2 [a] ; A ## _2 [a] = A ## _2 [b] ; A ## _2 [b] = t2 ; \
}

#define GB_partition GB_partition_3
#define GB_quicksort GB_quicksort_3

#include "GB_qsort_template.c"

void GB_qsort_3         // sort array A of size 3-by-n, using 3 keys (A [0:2][])
(
    int64_t A_0 [ ],      // size n array
    int64_t A_1 [ ],      // size n array
    int64_t A_2 [ ],      // size n array
    const int64_t n
)
{ 
    uint64_t seed = n ;
    GB_quicksort (GB_arg (A), n, &seed) ;
}

