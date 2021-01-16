//------------------------------------------------------------------------------
// GB_omp.h: definitions using OpenMP in SuiteSparse:GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_OMP_H
#define GB_OMP_H

//------------------------------------------------------------------------------
// determine the OpenMP version
//------------------------------------------------------------------------------

#if GB_MICROSOFT

    // MS Visual Studio supports OpenMP 2.0, and does not have the atomic
    // capture clause.  However, it has interlocked compare/exchange functions
    // that are used instead (see GB_atomics.h).
    #include <intrin.h>

#elif defined ( _OPENMP )

    // All other compilers must either support OpenMP 3.1 or later, or not use
    // OpenMP at all.
    #if _OPENMP < 201107
        #error "OpenMP 3.1 or later required (recompile without OpenMP)"
    #endif

#endif

//------------------------------------------------------------------------------
// OpenMP definitions
//------------------------------------------------------------------------------

#if defined ( _OPENMP )

    #include <omp.h>
    #define GB_OPENMP_MAX_THREADS       omp_get_max_threads ( )
    #define GB_OPENMP_GET_NUM_THREADS   omp_get_num_threads ( )
    #define GB_OPENMP_GET_WTIME         omp_get_wtime ( )
    #define GB_OPENMP_GET_THREAD_ID     omp_get_thread_num ( )

#else

    #define GB_OPENMP_MAX_THREADS       (1)
    #define GB_OPENMP_GET_NUM_THREADS   (1)
    #define GB_OPENMP_GET_WTIME         (0)
    #define GB_OPENMP_GET_THREAD_ID     (0)

#endif

#endif

