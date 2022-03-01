//------------------------------------------------------------------------------
// GraphBLAS/Demo/Include/simple_rand.h: a very simple random number generator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

//  Since the simple_rand ( ) function is replicated from the POSIX.1-2001
//  standard, no copyright claim is intended for this specific file.  The
//  copyright statement above applies to all of SuiteSparse:GraphBLAS, not
//  this file.

#ifndef SIMPLE_RAND_H
#define SIMPLE_RAND_H

#ifndef GB_PUBLIC
// Exporting/importing symbols for Microsoft Visual Studio
#if ( _MSC_VER && !__INTEL_COMPILER )
#ifdef GB_LIBRARY
// compiling SuiteSparse:GraphBLAS itself, exporting symbols to user apps
#define GB_PUBLIC extern __declspec ( dllexport )
#else
// compiling the user application, importing symbols from SuiteSparse:GraphBLAS
#define GB_PUBLIC extern __declspec ( dllimport )
#endif
#else
// for other compilers
#define GB_PUBLIC extern
#endif
#endif

#include <stdint.h>

#define SIMPLE_RAND_MAX 32767

// return a random number between 0 and SIMPLE_RAND_MAX
GB_PUBLIC
uint64_t simple_rand (void) ;

// set the seed
GB_PUBLIC
void simple_rand_seed (uint64_t seed) ;

// get the seed
GB_PUBLIC
uint64_t simple_rand_getseed (void) ;

// return a random double between 0 and 1, inclusive
GB_PUBLIC
double simple_rand_x ( ) ;

// return a random uint64_t
GB_PUBLIC
uint64_t simple_rand_i ( ) ;

#endif
