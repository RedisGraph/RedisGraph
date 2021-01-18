//------------------------------------------------------------------------------
// GB_select.h: definitions for GrB_select and related functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_SELECT_H
#define GB_SELECT_H
#include "GB.h"
#include "GB_is_nonzero.h"

GrB_Info GB_select          // C<M> = accum (C, select(A,k)) or select(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // descriptor for M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GxB_SelectOp op,          // operator to select the entries
    const GrB_Matrix A,             // input matrix
    const GxB_Scalar Thunk_in,      // optional input for select operator
    const bool A_transpose,         // A matrix descriptor
    GB_Context Context
) ;

GrB_Info GB_selector
(
    GrB_Matrix *Chandle,        // output matrix, NULL to modify A in-place
    GB_Select_Opcode opcode,    // selector opcode
    const GxB_SelectOp op,      // user operator
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    int64_t ithunk,             // (int64_t) Thunk, if Thunk is NULL
    const GxB_Scalar Thunk,     // optional input for select operator
    GB_Context Context
) ;

GrB_Info GB_bitmap_selector
(
    GrB_Matrix *Chandle,        // output matrix, NULL to modify A in-place
    GB_Select_Opcode opcode,    // selector opcode
    const GxB_select_function user_select,      // user select function
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    const int64_t ithunk,       // (int64_t) Thunk, if Thunk is NULL
    const GB_void *GB_RESTRICT xthunk,
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// compiler diagnostics
//------------------------------------------------------------------------------

// Some parameters are unused for some uses of the Generated/GB_sel_* functions
#include "GB_unused.h"

#endif

