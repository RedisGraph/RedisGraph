//------------------------------------------------------------------------------
// GB_atomics.h: definitions for atomic pragmas
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_ATOMICS_H
#define GB_ATOMICS_H
#include "GB.h"

#if GB_MICROSOFT

// FUTURE::: atomics with MS Visual Studio
#define GB_ATOMIC_READ
#define GB_ATOMIC_WRITE
#define GB_ATOMIC_UPDATE  GB_PRAGMA (omp atomic)
#define GB_ATOMIC_CAPTURE GB_PRAGMA (omp atomic)

#else

#define GB_ATOMIC_READ    GB_PRAGMA (omp atomic read)
#define GB_ATOMIC_WRITE   GB_PRAGMA (omp atomic write)
#define GB_ATOMIC_UPDATE  GB_PRAGMA (omp atomic update)
#define GB_ATOMIC_CAPTURE GB_PRAGMA (omp atomic capture)

#endif
#endif

