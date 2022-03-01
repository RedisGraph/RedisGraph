//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy_generic_firstj32.c: C=A*B, C bitmap/full, FIRSTJ mult
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is bitmap/full
// multiply op is GxB_FIRSTJ_INT32, GxB_FIRSTJ1_INT32,
// GxB_SECONDI_INT32, or GxB_SECONDI1_INT32

#define GB_AXB_SAXPY_GENERIC_METHOD GB_bitmap_AxB_saxpy_generic_firstj32 
#define C_IS_SPARSE_OR_HYPERSPARSE  0
#define OP_IS_POSITIONAL            1
#define FLIPXY                      0
#define OP_IS_INT64                 0
#define OP_IS_FIRSTI                0
#define OP_IS_FIRSTJ                1
#define OP_IS_FIRST                 0
#define OP_IS_SECOND                0

#include "GB_AxB_saxpy_generic_method.c"

