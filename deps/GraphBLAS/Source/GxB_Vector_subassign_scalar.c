//------------------------------------------------------------------------------
// GxB_Vector_subassign_[SCALAR]: assign scalar to vector, via scalar expansion
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Assigns a single scalar to a subvector, w(Rows)<M> = accum(w(Rows),x)
// The scalar x is implicitly expanded into a vector u of size nRows-by-1,
// with each entry in u equal to x.

// The actual work is done in GB_subassign_scalar.c.

#include "GB_subassign.h"

#define GB_ASSIGN_SCALAR(type,T,ampersand)                                     \
GrB_Info GB_EVAL2 (GXB (Vector_subassign_), T) /* w(I)<M> = accum (w(I),x)  */ \
(                                                                              \
    GrB_Vector w,                   /* input/output vector for results      */ \
    const GrB_Vector M,             /* optional mask for w(Rows)            */ \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(w(Rows),x)*/ \
    type x,                         /* scalar to assign to w(Rows)          */ \
    const GrB_Index *Rows,          /* row indices                          */ \
    GrB_Index nRows,                /* number of row indices                */ \
    const GrB_Descriptor desc       /* descriptor for w(Rows) and M         */ \
)                                                                              \
{                                                                              \
    GB_WHERE (w, "GxB_Vector_subassign_" GB_STR(T)                             \
        " (w, M, accum, x, Rows, nRows, desc)") ;                              \
    GB_BURBLE_START ("GxB_subassign") ;                                        \
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;                                          \
    GB_RETURN_IF_FAULTY (M) ;                                                  \
    ASSERT (GB_VECTOR_OK (w)) ;                                                \
    ASSERT (GB_IMPLIES (M != NULL, GB_VECTOR_OK (M))) ;                        \
    GrB_Info info = (GB_subassign_scalar ((GrB_Matrix) w, (GrB_Matrix) M,      \
        accum, ampersand x, GB_## T ## _code, Rows, nRows, GrB_ALL, 1, desc,   \
        Context)) ;                                                            \
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

