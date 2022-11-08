//------------------------------------------------------------------------------
// GB_cpu_features.h: cpu features for GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The following can be optionally #define'd at compile-time:
//
//  GBX86:  1 if the target architecture is x86_64,
//          0 if the target architecture is not x86_64.
//          default: #define'd below.
//
//  GBAVX2: 1 if the target architecture is x86_64 and supports AVX2,
//          0 otherwise.
//          default: left undefined and cpu_features/GetX86Info is used
//          to determine this feature at run-time.
//
//  GBAVX512F: 1 if the target architecture is x86_64 and supports AVX512F
//          0 otherwise.
//          default: left undefined and cpu_features/GetX86Info is used
//          to determine this feature at run-time.
//
//  GBNCPUFEAT: if #define'd then the Google cpu_features package is not used.
//          The run-time tests for AVX2 and AVX512F are replaced with
//          compile-time tests, using GBAVX2, and GBAVX512F.  If GBAVX2 or
//          GBAVX512F macros are not #define'd externally by the build system,
//          then no AVX acceleration is used.  default: not #define'd (using
//          Google's cpu_features).

#ifndef GB_CPU_FEATURES_H
#define GB_CPU_FEATURES_H

//------------------------------------------------------------------------------
// GB_compiler.h: determine the compiler and architecture
//------------------------------------------------------------------------------

#include "GB_compiler.h"

//------------------------------------------------------------------------------
// determine the target architecture
//------------------------------------------------------------------------------

#if !defined ( GBX86 )

    #if ( defined (_M_X64) || defined (__x86_64__)) && \
        ! ( defined (__CLR_VER) || defined (__pnacl__) )
    // the target architecture is x86_64, and not a virtual machine
    #define GBX86 1
    #else
    #define GBX86 0
    #endif

#endif

//------------------------------------------------------------------------------
// rely on Google's cpu_features package for run-time tests
//------------------------------------------------------------------------------

#if GB_COMPILER_MSC || GB_COMPILER_NVCC || GB_COMPILER_MINGW
// entirely disable cpu_features for MS Visual Studio, nvcc, and MinGW
#undef  GBNCPUFEAT
#define GBNCPUFEAT 1
#endif

#if !defined ( GBNCPUFEAT )

    #include "cpu_features_macros.h"
    #define STACK_LINE_READER_BUFFER_SIZE 1024

    #if GBX86
    // Intel x86 (also AMD): other architectures are not exploited
    #include "cpuinfo_x86.h"
    #endif

#endif

#endif

