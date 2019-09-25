//------------------------------------------------------------------------------
// GB_unused.h: pragmas to disable compiler warnings
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Because of the code generation mechanisms used, these compiler warnings are
// not avoidable, so disable them.

#ifndef GB_UNUSED_H
#define GB_UNUSED_H

#if defined ( __INTEL_COMPILER )
// disable icc -w3 warnings
#pragma warning (disable: 177 593)
#elif defined ( __GNUC__ )
// disable gcc -Wall -Wextra -Wpedantic warnings
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#endif

