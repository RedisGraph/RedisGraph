//------------------------------------------------------------------------------
// GB_AxB_saxpy3_generic_secondj32.c: C=A*B, C sparse/hyper, SECONDJ multiplier
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is sparse/hyper
// multiply op is GxB_SECONDJ_INT32 or GxB_SECONDJ1_INT32

#define GB_AXB_SAXPY_GENERIC_METHOD GB_AxB_saxpy3_generic_secondj32 
#define C_IS_SPARSE_OR_HYPERSPARSE  1
#define OP_IS_POSITIONAL            1
#define FLIPXY                      0
#define OP_IS_INT64                 0
#define OP_IS_FIRSTI                0
#define OP_IS_FIRSTJ                0
#define OP_IS_FIRST                 0
#define OP_IS_SECOND                0

#include "GB_AxB_saxpy_generic_method.c"

