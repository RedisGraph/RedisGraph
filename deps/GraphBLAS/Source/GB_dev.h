//------------------------------------------------------------------------------
// GB_dev.h: definitions for code development
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_DEV_H
#define GB_DEV_H

//------------------------------------------------------------------------------
// code development settings: by default, all settings should be commented out
//------------------------------------------------------------------------------

// to turn on Debug for a single file of GraphBLAS, add '#define GB_DEBUG'
// just before the statement '#include "GB.h"'

// to turn on Debug for all of GraphBLAS, uncomment this line:
// (GraphBLAS will be exceedingly slow; this is for development only)
// #define GB_DEBUG

// to reduce code size and for faster time to compile, uncomment this line;
// GraphBLAS will be slower.  Alternatively, use cmake with -DGBCOMPACT=1.
// (GraphBLAS will be exceedingly slow; this is for development only)
// #define GBCOMPACT 1

// to turn on a very verbose memory trace
// (GraphBLAS will be exceedingly slow; this is for development only)
// #define GB_MEMDUMP

// By default, many internal temporary matrices use statically allocated
// headers to reduce the number of calls to malloc/free.  This works fine for
// matrices on the CPU, but the static headers do not get automatically
// transfered to the GPU.  Only dynamically allocated headers, allocated by
// rmm_wrap_malloc, get transfered.  Set this to 1 to turn off static headers
// (required for CUDA; see GB_static_headers.h).  Leave static headers
// enabled by default by leaving this commented out or setting GBNSTATIC to 0.
// #undef  GBNSTATIC
// #define GBNSTATIC 1

#endif

