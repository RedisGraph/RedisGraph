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
// -DGB_BURBLE=0.
#ifndef GB_BURBLE
#define GB_BURBLE 1
#endif

// to turn on Debug for all of GraphBLAS, uncomment this line:
// #define GB_DEBUG

// to reduce code size and for faster time to compile, uncomment this line;
// GraphBLAS will be slower.  Alternatively, use cmake with -DGBCOMPACT=1
// #define GBCOMPACT 1

// for code development only
// #define GB_DEVELOPER 1

//------------------------------------------------------------------------------
// notes on future work
//------------------------------------------------------------------------------

// FUTURE: can handle transpose of full or bitmap input matrices just by
// changing how they are accessed
// 
// FUTURE: add matrix I/O in binary format (see draft LAGraph_binread/binwrite)
// 
// For PageRank:
// 
//  FUTURE: constant-valued matrices/vectors (for r(:)=teleport)
//      probably coupled with lazy malloc/free of A->x when converting from
//      full (non-constant) to constant-valued.
//      need aggressive exploit of non-blocking mode, for x = sum (abs (t-r)),
//      or GrB_vxv dot product, with PLUS_ABSDIFF semiring

#endif

