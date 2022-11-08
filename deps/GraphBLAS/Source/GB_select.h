//------------------------------------------------------------------------------
// GB_select.h: definitions for GrB_select and related functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
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
    const GB_Operator op,           // GxB_SelectOp or GrB_IndexUnaryOp
    const GrB_Matrix A,             // input matrix
    const GrB_Scalar Thunk_in,      // optional input for select operator
    const bool A_transpose,         // A matrix descriptor
    GB_Context Context
) ;

GrB_Info GB_selector
(
    GrB_Matrix C,               // output matrix, NULL or static header
    GB_Opcode opcode,           // selector opcode
    const GB_Operator op,       // user GxB_SelectOp or GrB_IndexUnaryOp op
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    int64_t ithunk,             // (int64_t) Thunk, if Thunk is NULL
    const GrB_Scalar Thunk,     // optional input for select operator
    GB_Context Context
) ;

GrB_Info GB_bitmap_selector
(
    GrB_Matrix C,               // output matrix, static header
    const bool C_iso,           // if true, C is iso
    GB_Opcode opcode,           // selector/idxunop opcode
    const GB_Operator op,
    const bool flipij,          // if true, flip i and j for user operator
    GrB_Matrix A,               // input matrix
    const int64_t ithunk,       // (int64_t) Thunk, if Thunk is NULL
    const GB_void *restrict athunk,     // (A->type) Thunk
    const GB_void *restrict ythunk,     // (op->utype) Thunk
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// GB_iso_select: assign the iso value of C for GB_*selector
//------------------------------------------------------------------------------

static inline void GB_iso_select
(
    void *Cx,                       // output iso value
    const GB_Opcode opcode,         // selector opcode
    const void *athunk,             // thunk scalar, of size asize
    const void *Ax,                 // Ax [0] scalar, of size asize
    const GB_Type_code acode,       // the type code of Ax
    const size_t asize
)
{
    if (opcode == GB_EQ_ZERO_selop_code)
    { 
        // all entries in C are zero
        memset (Cx, 0, asize) ;
    }
    else if (opcode == GB_EQ_THUNK_selop_code)
    { 
        // all entries in C are equal to thunk
        memcpy (Cx, athunk, asize) ;
    }
    else if (opcode == GB_NONZERO_selop_code && acode == GB_BOOL_code)
    { 
        // all entries in C are true; C and A are boolean
        memset (Cx, 1, 1) ;
    }
    else
    { 
        // A and C are both iso
        memcpy (Cx, Ax, asize) ;
    }
}

//------------------------------------------------------------------------------
// compiler diagnostics
//------------------------------------------------------------------------------

// Some parameters are unused for some uses of the Generated2/GB_sel_* functions
#include "GB_unused.h"

#endif

