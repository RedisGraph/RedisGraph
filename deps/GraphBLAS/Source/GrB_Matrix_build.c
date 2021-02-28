//------------------------------------------------------------------------------
// GrB_Matrix_build: build a sparse GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_build.h"

#define GB_MATRIX_BUILD(type,T)                                               \
GrB_Info GrB_Matrix_build_ ## T     /* build a matrix from (I,J,X) tuples */  \
(                                                                             \
    GrB_Matrix C,                   /* matrix to build                    */  \
    const GrB_Index *I,             /* array of row indices of tuples     */  \
    const GrB_Index *J,             /* array of column indices of tuples  */  \
    const type *X,                  /* array of values of tuples          */  \
    GrB_Index nvals,                /* number of tuples                   */  \
    const GrB_BinaryOp dup          /* binary op to assemble duplicates   */  \
)                                                                             \
{                                                                             \
    GB_WHERE ("GrB_Matrix_build_" GB_STR(T) " (C, I, J, X, nvals, dup)") ;    \
    GB_BURBLE_START ("GrB_Matrix_build") ;                                    \
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;                                         \
    GrB_Info info = GB_matvec_build (C, I, J, X, nvals, dup,                  \
        GB_ ## T ## _code, true, Context) ;                                   \
    GB_BURBLE_END ;                                                           \
    return (info) ;                                                           \
}

GB_MATRIX_BUILD (bool     , BOOL   )
GB_MATRIX_BUILD (int8_t   , INT8   )
GB_MATRIX_BUILD (uint8_t  , UINT8  )
GB_MATRIX_BUILD (int16_t  , INT16  )
GB_MATRIX_BUILD (uint16_t , UINT16 )
GB_MATRIX_BUILD (int32_t  , INT32  )
GB_MATRIX_BUILD (uint32_t , UINT32 )
GB_MATRIX_BUILD (int64_t  , INT64  )
GB_MATRIX_BUILD (uint64_t , UINT64 )
GB_MATRIX_BUILD (float    , FP32   )
GB_MATRIX_BUILD (double   , FP64   )
GB_MATRIX_BUILD (void     , UDT    )

