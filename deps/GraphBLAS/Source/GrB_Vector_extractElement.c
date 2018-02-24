//------------------------------------------------------------------------------
// GrB_Vector_extractElement: extract a single entry from a vector, x = v(i,j)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Extract a single entry, x = v(i), typecasting from the type
// of v to the type of x, as needed.

// Returns GrB_SUCCESS if A(i,j) is present, and sets x to its value.
// Returns GrB_NO_VALUE if A(i,j) is not present, and x is unmodified.

#include "GB.h"

#define EXTRACT(type,T)                                                       \
GrB_Info GrB_Vector_extractElement_ ## T     /* x = v(i) */                   \
(                                                                             \
    type *x,                            /* extracted scalar                */ \
    const GrB_Vector v,                 /* vector to extract a scalar from */ \
    const GrB_Index i                   /* row index                       */ \
)                                                                             \
{                                                                             \
    RETURN_IF_NULL_OR_UNINITIALIZED (v) ;                                     \
    WHERE ("GrB_Vector_extractElement_" GB_STR(T) " (x, v, i)") ;             \
    GrB_Info info = GB_extractElement (x, GB_ ## T ## _code,                  \
       (GrB_Matrix) v, i, 0) ;                                                \
    REPORT_VECTOR (info) ;                                                    \
    return (info) ;                                                           \
}

EXTRACT (bool     , BOOL) ;
EXTRACT (int8_t   , INT8) ;
EXTRACT (uint8_t  , UINT8) ;
EXTRACT (int16_t  , INT16) ;
EXTRACT (uint16_t , UINT16) ;
EXTRACT (int32_t  , INT32) ;
EXTRACT (uint32_t , UINT32) ;
EXTRACT (int64_t  , INT64) ;
EXTRACT (uint64_t , UINT64) ;
EXTRACT (float    , FP32) ;
EXTRACT (double   , FP64) ;
EXTRACT (void     , UDT) ;

#undef EXTRACT

