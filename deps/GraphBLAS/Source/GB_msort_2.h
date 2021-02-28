//------------------------------------------------------------------------------
// GB_msort_2.h: definitions for GB_msort_2.c
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A parallel mergesort of an array of 2-by-n integers.  Each key consists
// of two integers.

#include "GB_sort.h"

//------------------------------------------------------------------------------
// prototypes only needed for GB_msort_2
//------------------------------------------------------------------------------

void GB_merge_parallel_2                // parallel merge
(
    int64_t *GB_RESTRICT S_0,              // output of length nbigger + nsmaller
    int64_t *GB_RESTRICT S_1,
    const int64_t *GB_RESTRICT Bigger_0,   // Bigger [0..nbigger-1]
    const int64_t *GB_RESTRICT Bigger_1,
    const int64_t nbigger,
    const int64_t *GB_RESTRICT Smaller_0,  // Smaller [0..nsmaller-1]
    const int64_t *GB_RESTRICT Smaller_1,
    const int64_t nsmaller
) ;

void GB_merge_select_2      // parallel or sequential merge of 2-by-n arrays
(
    int64_t *GB_RESTRICT S_0,              // output of length nleft+nright
    int64_t *GB_RESTRICT S_1,
    const int64_t *GB_RESTRICT Left_0,     // Left [0..nleft-1]
    const int64_t *GB_RESTRICT Left_1,
    const int64_t nleft,
    const int64_t *GB_RESTRICT Right_0,    // Right [0..nright-1]
    const int64_t *GB_RESTRICT Right_1,
    const int64_t nright
) ;

void GB_mergesort_2 // sort array A of size 2-by-n, using 2 keys (A [0:1][])
(
    int64_t *GB_RESTRICT A_0,      // size n array
    int64_t *GB_RESTRICT A_1,      // size n array
    int64_t *GB_RESTRICT W_0,      // size n array, workspace
    int64_t *GB_RESTRICT W_1,      // size n array, workspace
    const int64_t n
) ;

