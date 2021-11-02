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

#ifndef LAGRAPH_PLATFORM_H
#define LAGRAPH_PLATFORM_H

//==============================================================================
// include files
//==============================================================================

#include <limits.h>

//==============================================================================

#if defined ( _OPENMP )
    #include <omp.h>
#endif

#if defined ( __linux__ ) || defined ( __GNU__ )
    #include <sys/time.h>
#endif

#if defined ( __MACH__ ) && defined ( __APPLE__ )
    #include <mach/clock.h>
    #include <mach/mach.h>
#endif

#if !defined(__cplusplus)
    #define LAGRAPH_RESTRICT restrict
#elif defined(_MSC_BUILD) || defined(__clang__) || defined(__GNUC__) || defined(__INTEL_COMPILER)
    #define LAGRAPH_RESTRICT __restrict
#else
    #define LAGRAPH_RESTRICT
#endif

//==============================================================================
// GraphBLAS platform specifics

// vanilla vs SuiteSparse:
#if !defined ( LG_VANILLA ) && defined ( GxB_SUITESPARSE_GRAPHBLAS )
    // use SuiteSparse, and its GxB* extensions
    #define LG_SUITESPARSE 1
#else
    // use any GraphBLAS library (possibly SuiteSparse) but with no GxB*
    #define LG_SUITESPARSE 0
#endif

// what is the correct system 64-bit maximum integer?
#if LG_SUITESPARSE
    // SuiteSparse: use GxB_INDEX_MAX
    #define LAGRAPH_INDEX_MAX GxB_INDEX_MAX
#else
    // Note by Tim: ULONG_MAX will break all kinds of things, like
    // malloc/calloc/realloc/free wrappers, which guard against size_t
    // overflow, but will fail unless RSIZE_MAX is used (at least).
    // SIZE_MAX is the largest size_t, and INT64_MAX is the
    // largest signed int64_t.  However, the ANSI C11 standard recommends
    // a definition of RSIZE_MAX, which guards against overflow, as something
    // smaller than SIZE_MAX.  So instead of this value:

    // #define LAGRAPH_INDEX_MAX ULONG_MAX

    // I recommend just using 2^60.
    // This definition is the same as GxB_INDEX_MAX in SuiteSparse:GraphBLAS:
    #define LAGRAPH_INDEX_MAX ((GrB_Index) (1ULL << 60))
#endif

#endif  // LAGRAPH_PLATFORM_H
