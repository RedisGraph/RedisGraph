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
    GrB_Matrix C,                   // output matrix, static header
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

void GB_AxB_saxpy_sparsity          // determine C_sparsity and method to use
(
    // output:
    int *C_sparsity,                // sparsity structure of C
    int *saxpy_method,              // saxpy method to use
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
    const bool M_in_place,          // ignored if C is bitmap
    const GrB_Matrix A,
    bool A_is_pattern,
    const GrB_Matrix B,
    bool B_is_pattern,
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const int saxpy_method,         // saxpy3, or bitmap method
    // for saxpy3 only:
    GB_saxpy3task_struct *restrict SaxpyTasks, // NULL if C is bitmap
    int ntasks,
    int nfine,
    int nthreads,
    const int do_sort,              // if true, sort in saxpy3
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// saxpy methods
//------------------------------------------------------------------------------

#define GB_SAXPY_METHOD_3 3
#define GB_SAXPY_METHOD_BITMAP 5
#define GB_SAXPY_METHOD_ISO_FULL 6

#endif

