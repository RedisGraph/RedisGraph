//------------------------------------------------------------------------------
// GrB_Matrix_setElement: set an entry in a matrix, C(row,col) = x
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Set a single entry in a matrix, C(row,col) = x in MATLAB notation,
// typecasting from the type of x to the type of C, as needed.

#include "GB.h"

#define GB_SET(prefix,type,T,ampersand)                                     \
GrB_Info prefix ## Matrix_setElement_ ## T    /* C (row,col) = x */         \
(                                                                           \
    GrB_Matrix C,                       /* matrix to modify               */\
    type x,                             /* scalar to assign to C(row,col) */\
    GrB_Index row,                      /* row index                      */\
    GrB_Index col                       /* column index                   */\
)                                                                           \
{                                                                           \
    GB_WHERE (C, GB_STR(prefix) "Matrix_setElement_" GB_STR(T)              \
        " (C, row, col, x)") ;                                              \
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;                                       \
    return (GB_setElement (C, ampersand x, row, col, GB_ ## T ## _code,     \
        Context)) ;                                                         \
}

GB_SET (GrB_, bool      , BOOL   , &)
GB_SET (GrB_, int8_t    , INT8   , &)
GB_SET (GrB_, int16_t   , INT16  , &)
GB_SET (GrB_, int32_t   , INT32  , &)
GB_SET (GrB_, int64_t   , INT64  , &)
GB_SET (GrB_, uint8_t   , UINT8  , &)
GB_SET (GrB_, uint16_t  , UINT16 , &)
GB_SET (GrB_, uint32_t  , UINT32 , &)
GB_SET (GrB_, uint64_t  , UINT64 , &)
GB_SET (GrB_, float     , FP32   , &)
GB_SET (GrB_, double    , FP64   , &)
GB_SET (GxB_, GxB_FC32_t, FC32   , &)
GB_SET (GxB_, GxB_FC64_t, FC64   , &)
GB_SET (GrB_, void *    , UDT    ,  )

