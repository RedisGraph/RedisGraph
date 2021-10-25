//------------------------------------------------------------------------------
// GB_dev.h: definitions for code development
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
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
#define GB_DEBUG

// to reduce code size and for faster time to compile, uncomment this line;
// GraphBLAS will be slower.  Alternatively, use cmake with -DGBCOMPACT=1.
// (GraphBLAS will be exceedingly slow; this is for development only)
// #define GBCOMPACT 1

// to turn on a very verbose memory trace
// (GraphBLAS will be exceedingly slow; this is for development only)
// #define GB_MEMDUMP

//------------------------------------------------------------------------------
// notes on future work
//------------------------------------------------------------------------------

// FUTURE: transpose full or bitmap inputs by changing how they are accessed

// FUTURE: add matrix I/O in binary format (see draft LAGraph_binread/binwrite)

// FUTURE: DIFF1, DIFF2 binary operators, GxB_vxv
//
//      add binary operators:
//          DIFF1(x,y) = abs(x-y), for all types
//          DIFF2(x,y) = (x-y)^2 for real types, (x-y)*conj(x-y) for complex
//      to compute x = sum (abs (t-r)), with PLUS_DIFF1 semiring
//
//          1-norm of a x-y:  PLUS_DIFF1 with GxB_vtxv
//          2-norm of a x-y:  PLUS_DIFF2 with GxB_vtxv, the sqrt of the scalar
//          +inf-norm of a x-y:  MAX_DIFF1 with GxB_vtxv
//          -inf-norm of a x-y:  MIN_DIFF1 with GxB_vtxv
//
//      to compute these for just a vector x, use y as all-zero iso full
//
//      GxB_vtxv : inner product of 2 vectors, result is a GxB_Scalar
//      GxB_vxvt : outer product of 2 vectors, result is a GrB_Matrix

// FUTURE: do O(1)-time typecasting for iso matrices, if their types don't match
//      the operator.  Do this after WAIT on the inputs, since that may change
//      the iso property.

#endif

