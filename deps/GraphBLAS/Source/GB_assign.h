//------------------------------------------------------------------------------
// GB_assign.h: definitions for GB_assign and related functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_ASSIGN_H
#define GB_ASSIGN_H
#include "GB_ij.h"

GrB_Info GB_assign                  // C<M>(Rows,Cols) += A or A'
(
    GrB_Matrix C,                   // input/output matrix for results
    bool C_replace,                 // descriptor for C
    const GrB_Matrix M_in,          // optional mask for C
    const bool Mask_comp,           // true if mask is complemented
    const bool Mask_struct,         // if true, use the only structure of M
    bool M_transpose,               // true if the mask should be transposed
    const GrB_BinaryOp accum,       // optional accum for accum(C,T)
    const GrB_Matrix A_in,          // input matrix
    bool A_transpose,               // true if A is transposed
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    const bool scalar_expansion,    // if true, expand scalar to A
    const void *scalar,             // scalar to be expanded
    const GB_Type_code scalar_code, // type code of scalar to expand
    const bool col_assign,          // true for GrB_Col_assign
    const bool row_assign,          // true for GrB_Row_assign
    GB_Context Context
) ;

GrB_Info GB_assign_scalar           // C<M>(Rows,Cols) += x
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // mask for C(Rows,Cols), unused if NULL
    const GrB_BinaryOp accum,       // accum for Z=accum(C(Rows,Cols),T)
    const void *scalar,             // scalar to assign to C(Rows,Cols)
    const GB_Type_code scalar_code, // type code of scalar to assign
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows,          // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols,          // number of column indices
    const GrB_Descriptor desc,      // descriptor for C and M
    GB_Context Context
) ;

void GB_assign_zombie1
(
    GrB_Matrix C,
    const int64_t j,
    GB_Context Context
) ;

void GB_assign_zombie2
(
    GrB_Matrix C,
    const int64_t i,
    GB_Context Context
) ;

void GB_assign_zombie3
(
    GrB_Matrix Z,
    const GrB_Matrix M,
    const bool Mask_comp,
    const bool Mask_struct,         // if true, use the only structure of M
    const int64_t j,
    const GrB_Index *I,
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    GB_Context Context
) ;

void GB_assign_zombie4
(
    GrB_Matrix Z,
    const GrB_Matrix M,
    const bool Mask_comp,
    const bool Mask_struct,         // if true, use the only structure of M
    const int64_t i,
    const GrB_Index *J,
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    GB_Context Context
) ;

GrB_Info GB_assign_zombie5
(
    GrB_Matrix Z,
    const GrB_Matrix M,
    const bool Mask_comp,
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Index *I,
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    GB_Context Context
) ;

#endif
