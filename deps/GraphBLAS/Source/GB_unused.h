//------------------------------------------------------------------------------
// GB_unused.h: pragmas to disable compiler warnings
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Because of the code generation mechanisms used, these compiler warnings are
// not avoidable, so disable them.

#ifndef GB_UNUSED_H
#define GB_UNUSED_H

#if ( _MSC_VER && !__INTEL_COMPILER )
// disable MS Visual Studio warnings
#pragma warning (disable: 4101 )
#elif defined ( __INTEL_COMPILER )
// disable icc -w3 warnings
#pragma warning (disable: 177 593)
#elif defined ( __GNUC__ )
// disable gcc -Wall -Wextra -Wpedantic warnings
#if (__GNUC__ > 4)
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#endif

