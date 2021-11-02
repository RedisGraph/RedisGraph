//------------------------------------------------------------------------------
// LAGraph_platform.h: wrappers around platform-specific GraphBLAS extensions
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
//
// See additional acknowledgments in the LICENSE file,
// or contact permission@sei.cmu.edu for the full terms.

//------------------------------------------------------------------------------

// Abstract away as much about implementation specific aspects of GraphBLAS
// distributions and operating systems as possible.

//------------------------------------------------------------------------------

#ifndef LAGRAPH_INTERNAL_H
#define LAGRAPH_INTERNAL_H

//==============================================================================
// include files
//==============================================================================

#include "GraphBLAS.h"
#include "LAGraph_platform.h"

//------------------------------------------------------------------------------
// simple and portable random number generator
//------------------------------------------------------------------------------

#define LAGRAPH_RANDOM15_MAX 32767
#define LAGRAPH_RANDOM60_MAX (LAGRAPH_INDEX_MAX-1)

// return a random number between 0 and LAGRAPH_RANDOM15_MAX
static inline GrB_Index LAGraph_Random15 (uint64_t *seed)
{
   (*seed) = (*seed) * 1103515245 + 12345 ;
   return (((*seed) / 65536) % (LAGRAPH_RANDOM15_MAX + 1)) ;
}

// return a random uint64_t, in range 0 to LAGRAPH_RANDOM60_MAX
static inline GrB_Index LAGraph_Random60 (uint64_t *seed)
{
    GrB_Index i = LAGraph_Random15 (seed) ;
    i = LAGRAPH_RANDOM15_MAX * i + LAGraph_Random15 (seed) ;
    i = LAGRAPH_RANDOM15_MAX * i + LAGraph_Random15 (seed) ;
    i = LAGRAPH_RANDOM15_MAX * i + LAGraph_Random15 (seed) ;
    i = i % (LAGRAPH_RANDOM60_MAX + 1) ;
    return (i) ;
}

//------------------------------------------------------------------------------
// parallel sorting methods
//------------------------------------------------------------------------------

int LAGraph_Sort1    // sort array A of size n
(
    int64_t *LAGRAPH_RESTRICT A_0,   // size n array
    const int64_t n,
    int nthreads,               // # of threads to use
    char *msg
) ;

int LAGraph_Sort2    // sort array A of size 2-by-n, using 2 keys (A [0:1][])
(
    int64_t *LAGRAPH_RESTRICT A_0,   // size n array
    int64_t *LAGRAPH_RESTRICT A_1,   // size n array
    const int64_t n,
    int nthreads,               // # of threads to use
    char *msg
) ;

int LAGraph_Sort3    // sort array A of size 3-by-n, using 3 keys (A [0:2][])
(
    int64_t *LAGRAPH_RESTRICT A_0,   // size n array
    int64_t *LAGRAPH_RESTRICT A_1,   // size n array
    int64_t *LAGRAPH_RESTRICT A_2,   // size n array
    const int64_t n,
    int nthreads,               // # of threads to use
    char *msg
) ;

#endif // LAGRAPH_INTERNAL_H
