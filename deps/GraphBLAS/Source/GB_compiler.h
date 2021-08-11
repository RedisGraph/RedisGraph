//------------------------------------------------------------------------------
// GB_compiler.h: handle compiler variations
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_COMPILER_H
#define GB_COMPILER_H

//------------------------------------------------------------------------------
// compiler variations
//------------------------------------------------------------------------------

// Determine the restrict keyword, and whether or not variable-length arrays
// are supported.

#if ( _MSC_VER && !__INTEL_COMPILER )

    // Microsoft Visual Studio does not have the restrict keyword, but it does
    // support __restrict, which is equivalent.  Variable-length arrays are
    // not supported.  OpenMP tasks are not available, GraphBLAS no longer
    // uses OpenMP tasks.

    #define GB_MICROSOFT 1
    #define GB_HAS_VLA  0
    #if defined ( __cplusplus )
        // C++ does not have the restrict keyword
        #define restrict
    #else
        // C uses __restrict
        #define restrict __restrict
    #endif

#elif defined ( __cplusplus )

    #define GB_MICROSOFT 0
    #define GB_HAS_VLA  1
    // C++ does not have the restrict keyword
    #define restrict

#elif GxB_STDC_VERSION >= 199901L

    // ANSI C99 and later have the restrict keyword and variable-length arrays.
    #define GB_MICROSOFT 0
    #define GB_HAS_VLA  1

#else

    // ANSI C95 and earlier have neither
    #define GB_MICROSOFT 0
    #define GB_HAS_VLA  0
    #define restrict

#endif

//------------------------------------------------------------------------------
// Microsoft specific include files
//------------------------------------------------------------------------------

#if GB_MICROSOFT
#include <malloc.h>
#endif

//------------------------------------------------------------------------------
// PGI_COMPILER_BUG
//------------------------------------------------------------------------------

// If GraphBLAS is compiled with -DPGI_COMPILER_BUG, then a workaround is
// enabled for a bug in the PGI compiler.  The compiler does not correctly
// handle automatic arrays of variable size.

#ifdef PGI_COMPILER_BUG

    // override the ANSI C compiler to turn off variable-length arrays
    #undef  GB_HAS_VLA
    #define GB_HAS_VLA  0

#endif

//------------------------------------------------------------------------------
// OpenMP pragmas and tasks
//------------------------------------------------------------------------------

// GB_PRAGMA(x) becomes "#pragma x", but the way to do this depends on the
// compiler:
#if GB_MICROSOFT
    // MS Visual Studio is not ANSI C11 compliant, and uses __pragma:
    #define GB_PRAGMA(x) __pragma (x)
#else
    // ANSI C11 compilers use _Pragma:
    #define GB_PRAGMA(x) _Pragma (#x)
#endif

// construct pragmas for loop vectorization:
#if GB_MICROSOFT

    // no #pragma omp simd is available in MS Visual Studio
    #define GB_PRAGMA_SIMD
    #define GB_PRAGMA_SIMD_REDUCTION(op,s)

#else

    // create two kinds of SIMD pragmas:
    // GB_PRAGMA_SIMD becomes "#pragma omp simd"
    // GB_PRAGMA_SIMD_REDUCTION (+,cij) becomes
    // "#pragma omp simd reduction(+:cij)"
    #define GB_PRAGMA_SIMD GB_PRAGMA (omp simd)
    #define GB_PRAGMA_SIMD_REDUCTION(op,s) GB_PRAGMA (omp simd reduction(op:s))

#endif

#define GB_PRAGMA_IVDEP GB_PRAGMA(ivdep)

//------------------------------------------------------------------------------
// variable-length arrays
//------------------------------------------------------------------------------

// If variable-length arrays are not supported, user-defined types are limited
// in size to 128 bytes or less.  Many of the type-generic routines allocate
// workspace for a single scalar of variable size, using a statement:
//
//      GB_void aij [xsize] ;
//
// To support non-variable-length arrays in ANSI C95 or earlier, this is used:
//
//      GB_void aij [GB_VLA(xsize)] ;
//
// GB_VLA(xsize) is either defined as xsize (for ANSI C99 or later), or a fixed
// size of 128, in which case user-defined types
// are limited to a max of 128 bytes.

#if ( GB_HAS_VLA )

    // variable-length arrays are allowed
    #define GB_VLA(s) s

#else

    // variable-length arrays are not allowed
    #define GB_VLA_MAXSIZE 128
    #define GB_VLA(s) GB_VLA_MAXSIZE

#endif
#endif

