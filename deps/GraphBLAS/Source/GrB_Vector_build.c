//------------------------------------------------------------------------------
// GrB_Vector_build: build a sparse GraphBLAS vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

#define BUILD(type,T)                                                         \
GrB_Info GrB_Vector_build_ ## T     /* build a vector from (I,X) tuples   */  \
(                                                                             \
    GrB_Vector w,                   /* vector to build                    */  \
    const GrB_Index *I,             /* array of row indices of tuples     */  \
    const type *X,                  /* array of values of tuples          */  \
    const GrB_Index nvals,          /* number of tuples                   */  \
    const GrB_BinaryOp dup          /* binary op to assemble duplicates   */  \
)                                                                             \
{                                                                             \
    WHERE ("GrB_Vector_build_" GB_STR(T) " (w, I, X, nvals, dup)") ;          \
    RETURN_IF_NULL_OR_UNINITIALIZED (w) ;                                     \
    return (GB_build ((GrB_Matrix) w, I, NULL, X, nvals, dup,                 \
        GB_ ## T ## _code)) ;                                                 \
}

BUILD (bool     , BOOL   ) ;
BUILD (int8_t   , INT8   ) ;
BUILD (uint8_t  , UINT8  ) ;
BUILD (int16_t  , INT16  ) ;
BUILD (uint16_t , UINT16 ) ;
BUILD (int32_t  , INT32  ) ;
BUILD (uint32_t , UINT32 ) ;
BUILD (int64_t  , INT64  ) ;
BUILD (uint64_t , UINT64 ) ;
BUILD (float    , FP32   ) ;
BUILD (double   , FP64   ) ;
BUILD (void     , UDT    ) ;

#undef BUILD

