//------------------------------------------------------------------------------
// GrB_Vector_setElement: set an entry in a vector, w (row) = x
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Set a single scalar, w(row) = x, typecasting from the type of x to
// the type of w as needed.

#include "GB.h"

#define GB_SET(prefix,type,T,ampersand)                                     \
GrB_Info GB_EVAL3 (prefix, _Vector_setElement_, T)    /* w(row) = x */      \
(                                                                           \
    GrB_Vector w,                       /* vector to modify           */    \
    type x,                             /* scalar to assign to w(row) */    \
    GrB_Index row                       /* row index                  */    \
)                                                                           \
{                                                                           \
    GB_WHERE (w, "GrB_Vector_setElement_" GB_STR(T) " (w, x, row)") ;       \
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;                                       \
    ASSERT (GB_VECTOR_OK (w)) ;                                             \
    return (GB_setElement ((GrB_Matrix) w, ampersand x, row, 0,             \
        GB_ ## T ## _code, Context)) ;                                      \
}

GB_SET (GrB, bool      , BOOL   , &)
GB_SET (GrB, int8_t    , INT8   , &)
GB_SET (GrB, int16_t   , INT16  , &)
GB_SET (GrB, int32_t   , INT32  , &)
GB_SET (GrB, int64_t   , INT64  , &)
GB_SET (GrB, uint8_t   , UINT8  , &)
GB_SET (GrB, uint16_t  , UINT16 , &)
GB_SET (GrB, uint32_t  , UINT32 , &)
GB_SET (GrB, uint64_t  , UINT64 , &)
GB_SET (GrB, float     , FP32   , &)
GB_SET (GrB, double    , FP64   , &)
GB_SET (GxB, GxB_FC32_t, FC32   , &)
GB_SET (GxB, GxB_FC64_t, FC64   , &)
GB_SET (GrB, void *    , UDT    ,  )

