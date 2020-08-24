//------------------------------------------------------------------------------
// GB_sort.h: definitions for sorting functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// All of the GB_qsort_* functions are single-threaded, by design.  Both
// GB_msort_* functions are parallel.  None of these sorting methods are
// guaranteed to be stable, but they are always used in GraphBLAS with unique
// keys.

#include "GraphBLAS.h"
#include "LAGraph.h"

// from GB.h
typedef unsigned char GB_void ;

#ifndef GB_SORT_H
#define GB_SORT_H

#define GB_BASECASE (64 * 1024)

void GB_qsort_1a    // sort array A of size 1-by-n
(
    int64_t *LA_RESTRICT A_0,      // size n array
    const int64_t n
) ;

void GB_qsort_1b    // sort array A of size 2-by-n, using 1 key (A [0][])
(
    int64_t *LA_RESTRICT A_0,   // size n array
    GB_void *LA_RESTRICT A_1,   // size n array
    const size_t xsize,         // size of entries in A_1
    const int64_t n
) ;

void GB_qsort_2     // sort array A of size 2-by-n, using 2 keys (A [0:1][])
(
    int64_t *LA_RESTRICT A_0,      // size n array
    int64_t *LA_RESTRICT A_1,      // size n array
    const int64_t n
) ;

void GB_qsort_3     // sort array A of size 3-by-n, using 3 keys (A [0:2][])
(
    int64_t *LA_RESTRICT A_0,      // size n array
    int64_t *LA_RESTRICT A_1,      // size n array
    int64_t *LA_RESTRICT A_2,      // size n array
    const int64_t n
) ;

void GB_msort_2     // sort array A of size 2-by-n, using 2 keys (A [0:1][])
(
    int64_t *LA_RESTRICT A_0,   // size n array
    int64_t *LA_RESTRICT A_1,   // size n array
    int64_t *LA_RESTRICT W_0,   // size n array, workspace
    int64_t *LA_RESTRICT W_1,   // size n array, workspace
    const int64_t n,
    const int nthreads          // # of threads to use
) ;

void GB_msort_3     // sort array A of size 3-by-n, using 3 keys (A [0:2][])
(
    int64_t *LA_RESTRICT A_0,   // size n array
    int64_t *LA_RESTRICT A_1,   // size n array
    int64_t *LA_RESTRICT A_2,   // size n array
    int64_t *LA_RESTRICT W_0,   // size n array, workspace
    int64_t *LA_RESTRICT W_1,   // size n array, workspace
    int64_t *LA_RESTRICT W_2,   // size n array, workspace
    const int64_t n,
    const int nthreads          // # of threads to use
) ;

//------------------------------------------------------------------------------
// GB_lt_1: sorting comparator function, one key
//------------------------------------------------------------------------------

// A [a] and B [b] are keys of one integer.

// GB_lt_1 returns true if A [a] < B [b], for GB_qsort_1a and GB_qsort_1b

#define GB_lt_1(A_0, a, B_0, b)                                             \
    (A_0 [a] < B_0 [b])

//------------------------------------------------------------------------------
// GB_lt_2: sorting comparator function, two keys
//------------------------------------------------------------------------------

// A [a] and B [b] are keys of two integers.

// GB_lt_2 returns true if A [a] < B [b], for GB_qsort_2 and GB_msort_2

#define GB_lt_2(A_0, A_1, a, B_0, B_1, b)                                   \
(                                                                           \
    (A_0 [a] < B_0 [b]) ?                                                   \
    (                                                                       \
        true                                                                \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        (A_0 [a] == B_0 [b]) ?                                              \
        (                                                                   \
            /* primary key is the same; tie-break on the 2nd key */         \
            (A_1 [a] < B_1 [b])                                             \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            false                                                           \
        )                                                                   \
    )                                                                       \
)
    
//------------------------------------------------------------------------------
// GB_lt_3: sorting comparator function, three keys
//------------------------------------------------------------------------------

// A [a] and B [b] are keys of three integers.

// GB_lt_3 returns true if A [a] < B [b], for GB_qsort_3 and GB_msort_2

#define GB_lt_3(A_0, A_1, A_2, a, B_0, B_1, B_2, b)                         \
(                                                                           \
    (A_0 [a] < B_0 [b]) ?                                                   \
    (                                                                       \
        true                                                                \
    )                                                                       \
    :                                                                       \
    (                                                                       \
        (A_0 [a] == B_0 [b]) ?                                              \
        (                                                                   \
            /* primary key is the same; tie-break on the 2nd and 3rd key */ \
            GB_lt_2 (A_1, A_2, a, B_1, B_2, b)                              \
        )                                                                   \
        :                                                                   \
        (                                                                   \
            false                                                           \
        )                                                                   \
    )                                                                       \
)

//------------------------------------------------------------------------------
// random number generator for quicksort
//------------------------------------------------------------------------------

// return a random GrB_Index, in range 0 to 2^60
#define GB_RAND_MAX 32767

// return a random number between 0 and GB_RAND_MAX
static inline GrB_Index GB_rand15 (uint64_t *seed)
{ 
   (*seed) = (*seed) * 1103515245 + 12345 ;
   return (((*seed) / 65536) % (GB_RAND_MAX + 1)) ;
}

// return a random GrB_Index, in range 0 to 2^60
static inline GrB_Index GB_rand (uint64_t *seed)
{ 
    GrB_Index i = GB_rand15 (seed) ;
    i = GB_RAND_MAX * i + GB_rand15 (seed) ;
    i = GB_RAND_MAX * i + GB_rand15 (seed) ;
    i = GB_RAND_MAX * i + GB_rand15 (seed) ;
    return (i) ;
}

#endif

