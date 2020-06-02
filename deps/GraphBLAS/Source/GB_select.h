//------------------------------------------------------------------------------
// GB_select.h: definitions for GrB_select and related functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_SELECT_H
#define GB_SELECT_H
#include "GB.h"

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

//------------------------------------------------------------------------------
// GB_is_nonzero
//------------------------------------------------------------------------------

static inline bool GB_is_nonzero (const GB_void *value, int64_t size)
{ 
    for (int64_t i = 0 ; i < size ; i++)
    {
        if (value [i] != 0) return (true) ;
    }
    return (false) ;
}

//------------------------------------------------------------------------------
// compiler diagnostics
//------------------------------------------------------------------------------

// Tx unused for some uses of the Generated/GB_sel_* functions
#include "GB_unused.h"

#endif

