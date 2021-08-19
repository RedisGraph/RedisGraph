//------------------------------------------------------------------------------
// GxB_Matrix_subassign_[SCALAR]: assign to submatrix, via scalar expansion
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Assigns a single scalar to a submatrix:

// C(Rows,Cols)<M> = accum(C(Rows,Cols),x)

// The scalar x is implicitly expanded into a matrix A of size nRows-by-nCols,
// with each entry in A equal to x.

// Compare with GrB_Matrix_assign_scalar,
// which uses M and C_Replace differently.

#include "GB_subassign.h"

#define GB_ASSIGN_SCALAR(type,T,ampersand)                                     \
GrB_Info GB_EVAL2 (GXB (Matrix_subassign_), T) /* C(Rows,Cols)<M> += x      */ \
(                                                                              \
    GrB_Matrix C,                   /* input/output matrix for results      */ \
    const GrB_Matrix M,             /* optional mask for C(Rows,Cols)       */ \
    const GrB_BinaryOp accum,       /* accum for Z=accum(C(Rows,Cols),x)    */ \
    type x,                         /* scalar to assign to C(Rows,Cols)     */ \
    const GrB_Index *Rows,          /* row indices                          */ \
    GrB_Index nRows,                /* number of row indices                */ \
    const GrB_Index *Cols,          /* column indices                       */ \
    GrB_Index nCols,                /* number of column indices             */ \
    const GrB_Descriptor desc       /* descriptor for C(Rows,Cols) and M */    \
)                                                                              \
{                                                                              \
    GB_WHERE (C, "GxB_Matrix_subassign_" GB_STR(T)                             \
        " (C, M, accum, x, Rows, nRows, Cols, nCols, desc)") ;                 \
    GB_BURBLE_START ("GxB_Matrix_subassign " GB_STR(T)) ;                      \
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;                                          \
    GB_RETURN_IF_FAULTY (M) ;                                                  \
    GrB_Info info = GB_subassign_scalar (C, M, accum, ampersand x,             \
        GB_## T ## _code, Rows, nRows, Cols, nCols, desc, Context) ;           \
    GB_BURBLE_END ;                                                            \
    return (info) ;                                                            \
}

GB_ASSIGN_SCALAR (bool      , BOOL   , &)
GB_ASSIGN_SCALAR (int8_t    , INT8   , &)
GB_ASSIGN_SCALAR (uint8_t   , UINT8  , &)
GB_ASSIGN_SCALAR (int16_t   , INT16  , &)
GB_ASSIGN_SCALAR (uint16_t  , UINT16 , &)
GB_ASSIGN_SCALAR (int32_t   , INT32  , &)
GB_ASSIGN_SCALAR (uint32_t  , UINT32 , &)
GB_ASSIGN_SCALAR (int64_t   , INT64  , &)
GB_ASSIGN_SCALAR (uint64_t  , UINT64 , &)
GB_ASSIGN_SCALAR (float     , FP32   , &)
GB_ASSIGN_SCALAR (double    , FP64   , &)
GB_ASSIGN_SCALAR (GxB_FC32_t, FC32   , &)
GB_ASSIGN_SCALAR (GxB_FC64_t, FC64   , &)
GB_ASSIGN_SCALAR (void *    , UDT    ,  )

