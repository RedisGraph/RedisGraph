//------------------------------------------------------------------------------
// GxB_Scalar_setElement: set an entry in a GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Set a single scalar, s = x, typecasting from the type of x to
// the type of s as needed.

#include "GB.h"

#define GB_SET(type,T,ampersand)                                            \
GrB_Info GB_EVAL2 (GXB (Scalar_setElement_), T)    /* s = x */              \
(                                                                           \
    GxB_Scalar s,                       /* GxB_Scalar to modify       */    \
    type x                              /* user scalar to assign to s */    \
)                                                                           \
{                                                                           \
    GB_WHERE (s, "GxB_Scalar_setElement_" GB_STR(T) " (w, x)") ;            \
    GB_RETURN_IF_NULL_OR_FAULTY (s) ;                                       \
    ASSERT (GB_SCALAR_OK (s)) ;                                             \
    return (GB_setElement ((GrB_Matrix) s, ampersand x, 0, 0,               \
        GB_ ## T ## _code, Context)) ;                                      \
}

GB_SET (bool      , BOOL   , &)
GB_SET (int8_t    , INT8   , &)
GB_SET (int16_t   , INT16  , &)
GB_SET (int32_t   , INT32  , &)
GB_SET (int64_t   , INT64  , &)
GB_SET (uint8_t   , UINT8  , &)
GB_SET (uint16_t  , UINT16 , &)
GB_SET (uint32_t  , UINT32 , &)
GB_SET (uint64_t  , UINT64 , &)
GB_SET (float     , FP32   , &)
GB_SET (double    , FP64   , &)
GB_SET (GxB_FC32_t, FC32   , &)
GB_SET (GxB_FC64_t, FC64   , &)
GB_SET (void *    , UDT    ,  )

