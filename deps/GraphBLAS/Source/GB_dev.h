//------------------------------------------------------------------------------
// GB_dev.h: definitions for code development
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_DEV_H
#define GB_DEV_H

//------------------------------------------------------------------------------
// code development settings
//------------------------------------------------------------------------------

// to turn on Debug for a single file of GraphBLAS, add '#define GB_DEBUG'
// just before the statement '#include "GB.h"'

// set GB_BURBLE to 0 to disable diagnostic output, or compile with
// -DGB_BURBLE=0.  Enabling/disabling the burble has little effect on
// performance, unless GxB_set (GxB_BURBLE, true) is set.  In that case, a
// small drop in performance can occur because of the volume of output.  But
// with GxB_set (GxB_BURBLE, false), which is the default, the performance is
// not affected.
#ifndef GB_BURBLE
#define GB_BURBLE 1
#endif

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

#endif

