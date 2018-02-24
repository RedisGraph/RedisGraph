//------------------------------------------------------------------------------
// GrB_Vector_setElement: set an entry in a vector, w (i) = x
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Set a single scalar, w(i) = x, typecasting from the type of x to
// the type of w as needed.

#include "GB.h"

#define SET(type,T,ampersand)                                               \
GrB_Info GrB_Vector_setElement_ ## T    /* w(i) = x    */                   \
(                                                                           \
    GrB_Vector w,                       /* vector to modify           */    \
    const type x,                       /* scalar to assign to w(i)   */    \
    const GrB_Index i                   /* row index                  */    \
)                                                                           \
{                                                                           \
    WHERE ("GrB_Vector_setElement_" GB_STR(T) " (C, i, j, x)") ;            \
    RETURN_IF_NULL_OR_UNINITIALIZED (w) ;                                   \
    return (GB_setElement ((GrB_Matrix) w, ampersand x, i, 0,               \
        GB_ ## T ## _code)) ;                                               \
}

SET (bool     , BOOL   , &) ;
SET (int8_t   , INT8   , &) ;
SET (uint8_t  , UINT8  , &) ;
SET (int16_t  , INT16  , &) ;
SET (uint16_t , UINT16 , &) ;
SET (int32_t  , INT32  , &) ;
SET (uint32_t , UINT32 , &) ;
SET (int64_t  , INT64  , &) ;
SET (uint64_t , UINT64 , &) ;
SET (float    , FP32   , &) ;
SET (double   , FP64   , &) ;
SET (void *   , UDT    ,  ) ;

#undef SET

