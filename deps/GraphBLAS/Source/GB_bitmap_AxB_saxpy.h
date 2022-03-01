//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy: compute C=A*B, C<M>=A*B, or C<!M>=A*B; C bitmap or full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_AXB_BITMAP_SAXPY_H
#define GB_AXB_BITMAP_SAXPY_H
#include "GB_mxm.h"

GB_PUBLIC                           // for testing only
GrB_Info GB_bitmap_AxB_saxpy        // C = A*B where C is bitmap
(
    GrB_Matrix C,                   // output matrix, static header
    const bool C_iso,               // true if C is iso
    const GB_void *cscalar,         // iso value of C
    const GrB_Matrix M,             // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

#endif
