//------------------------------------------------------------------------------
// GrB_Matrix_assign_[SCALAR]: assign a scalar to matrix, via scalar expansion
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Assigns a single scalar to a matrix:

// C<M>(Rows,Cols) = accum(C(Rows,Cols),x)

// The scalar x is implicitly expanded into a matrix A of size nRows-by-nCols,
// with each entry in A equal to x.

// Compare with GxB_Matrix_subassign_scalar,
// which uses M and C_Replace differently.

// The actual work is done in GB_assign_scalar.c.

#include "GB_assign.h"

#define GB_ASSIGN(type,T,ampersand)                                            \
GrB_Info GrB_Matrix_assign_ ## T    /* C<M>(Rows,Cols) += x                 */ \
(                                                                              \
    GrB_Matrix C,                   /* input/output matrix for results      */ \
    const GrB_Matrix M,             /* optional mask for C                  */ \
    const GrB_BinaryOp accum,       /* accum for Z=accum(C(Rows,Cols),x)    */ \
    type x,                         /* scalar to assign to C(Rows,Cols)     */ \
    const GrB_Index *Rows,          /* row indices                          */ \
    GrB_Index nRows,                /* number of row indices                */ \
    const GrB_Index *Cols,          /* column indices                       */ \
    GrB_Index nCols,                /* number of column indices             */ \
    const GrB_Descriptor desc       /* descriptor for C and M               */ \
)                                                                              \
{                                                                              \
    GB_WHERE ("GrB_Matrix_assign_" GB_STR(T)                                   \
        " (C, M, accum, x, Rows, nRows, Cols, nCols, desc)") ;                 \
    GB_BURBLE_START ("GrB_assign") ;                                           \
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;                                          \
    GB_RETURN_IF_FAULTY (M) ;                                                  \
    GrB_Info info = GB_assign_scalar (C, M, accum, ampersand x,                \
        GB_## T ## _code, Rows, nRows, Cols, nCols, desc, Context) ;           \
    GB_BURBLE_END ;                                                            \
    return (info) ;                                                            \
}

GB_ASSIGN (bool     , BOOL   , &)
GB_ASSIGN (int8_t   , INT8   , &)
GB_ASSIGN (uint8_t  , UINT8  , &)
GB_ASSIGN (int16_t  , INT16  , &)
GB_ASSIGN (uint16_t , UINT16 , &)
GB_ASSIGN (int32_t  , INT32  , &)
GB_ASSIGN (uint32_t , UINT32 , &)
GB_ASSIGN (int64_t  , INT64  , &)
GB_ASSIGN (uint64_t , UINT64 , &)
GB_ASSIGN (float    , FP32   , &)
GB_ASSIGN (double   , FP64   , &)
GB_ASSIGN (void *   , UDT    ,  )

