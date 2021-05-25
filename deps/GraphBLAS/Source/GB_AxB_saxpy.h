//------------------------------------------------------------------------------
// GB_AxB_saxpy.h: definitions for GB_AxB_saxpy and related methods
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_AXB_SAXPY_H
#define GB_AXB_SAXPY_H
#include "GB.h"
#include "GB_AxB_saxpy3.h"

//------------------------------------------------------------------------------
// GB_AxB_saxpy
//------------------------------------------------------------------------------

GrB_Info GB_AxB_saxpy               // C = A*B using Gustavson/Hash/Bitmap
(
    GrB_Matrix *Chandle,            // output matrix (if not done in-place)
    const GrB_Matrix M,             // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, then mask was applied
    const GrB_Desc_Value AxB_method,
    const int do_sort,              // if nonzero, try to sort in saxpy3
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// GB_AxB_saxpy_sparsity: determine the sparsity of C
//------------------------------------------------------------------------------

int GB_AxB_saxpy_sparsity           // return the sparsity structure for C
(
    // input:
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input A matrix
    const GrB_Matrix B,             // input B matrix
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// GB_AxB_saxpy_generic: for any types and operators
//------------------------------------------------------------------------------

GrB_Info GB_AxB_saxpy_generic
(
    GrB_Matrix C,                   // any sparsity
    const GrB_Matrix M,
    bool Mask_comp,
    const bool Mask_struct,
    const bool M_dense_in_place,    // ignored if C is bitmap
    const GrB_Matrix A,
    bool A_is_pattern,
    const GrB_Matrix B,
    bool B_is_pattern,
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_saxpy3task_struct *GB_RESTRICT TaskList, // NULL if C is bitmap
    int ntasks,
    int nfine,
    int nthreads,
    const int do_sort,              // if nonzero, try to sort in saxpy3
    GB_Context Context
) ;

#endif

