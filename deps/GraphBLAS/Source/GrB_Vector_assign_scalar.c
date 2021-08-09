//------------------------------------------------------------------------------
// GrB_Vector_assign_[SCALAR]: assign scalar to vector, via scalar expansion
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Assigns a single scalar to a vector, w<M>(Rows) = accum(w(Rows),x)
// The scalar x is implicitly expanded into a vector u of size nRows-by-1,
// with each entry in u equal to x.

#include "GB_assign.h"

#define GB_ASSIGN_SCALAR(prefix,type,T,ampersand)                              \
GrB_Info GB_EVAL3 (prefix, _Vector_assign_, T) /* w<M>(Rows)=accum(w(Rows),x)*/\
(                                                                              \
    GrB_Vector w,                   /* input/output vector for results      */ \
    const GrB_Vector M,             /* optional mask for w                  */ \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(w(Rows),x)*/ \
    type x,                         /* scalar to assign to w(Rows)          */ \
    const GrB_Index *Rows,          /* row indices                          */ \
    GrB_Index nRows,                /* number of row indices                */ \
    const GrB_Descriptor desc       /* descriptor for w and mask            */ \
)                                                                              \
{                                                                              \
    GB_WHERE (w, "GrB_Vector_assign_" GB_STR(T)                                \
        " (w, M, accum, x, Rows, nRows, desc)") ;                              \
    GB_BURBLE_START ("GrB_assign") ;                                           \
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;                                          \
    GB_RETURN_IF_FAULTY (M) ;                                                  \
    ASSERT (GB_VECTOR_OK (w)) ;                                                \
    ASSERT (GB_IMPLIES (M != NULL, GB_VECTOR_OK (M))) ;                        \
    GrB_Info info = GB_assign_scalar ((GrB_Matrix) w, (GrB_Matrix) M, accum,   \
        ampersand x, GB_## T ## _code, Rows, nRows, GrB_ALL, 1, desc,          \
        Context) ;                                                             \
    GB_BURBLE_END ;                                                            \
    return (info) ;                                                            \
}

GB_ASSIGN_SCALAR (GrB, bool      , BOOL   , &)
GB_ASSIGN_SCALAR (GrB, int8_t    , INT8   , &)
GB_ASSIGN_SCALAR (GrB, uint8_t   , UINT8  , &)
GB_ASSIGN_SCALAR (GrB, int16_t   , INT16  , &)
GB_ASSIGN_SCALAR (GrB, uint16_t  , UINT16 , &)
GB_ASSIGN_SCALAR (GrB, int32_t   , INT32  , &)
GB_ASSIGN_SCALAR (GrB, uint32_t  , UINT32 , &)
GB_ASSIGN_SCALAR (GrB, int64_t   , INT64  , &)
GB_ASSIGN_SCALAR (GrB, uint64_t  , UINT64 , &)
GB_ASSIGN_SCALAR (GrB, float     , FP32   , &)
GB_ASSIGN_SCALAR (GrB, double    , FP64   , &)
GB_ASSIGN_SCALAR (GxB, GxB_FC32_t, FC32   , &)
GB_ASSIGN_SCALAR (GxB, GxB_FC64_t, FC64   , &)
GB_ASSIGN_SCALAR (GrB, void *    , UDT    ,  )

