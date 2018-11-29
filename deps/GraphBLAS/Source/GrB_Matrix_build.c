//------------------------------------------------------------------------------
// GrB_Matrix_build: build a sparse GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

#define GB_BUILD(type,T)                                                      \
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
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;                                         \
    return (GB_user_build (C, I, J, X, nvals, dup, GB_ ## T ## _code, true,   \
        Context)) ;                                                           \
}

GB_BUILD (bool     , BOOL   )
GB_BUILD (int8_t   , INT8   )
GB_BUILD (uint8_t  , UINT8  )
GB_BUILD (int16_t  , INT16  )
GB_BUILD (uint16_t , UINT16 )
GB_BUILD (int32_t  , INT32  )
GB_BUILD (uint32_t , UINT32 )
GB_BUILD (int64_t  , INT64  )
GB_BUILD (uint64_t , UINT64 )
GB_BUILD (float    , FP32   )
GB_BUILD (double   , FP64   )
GB_BUILD (void     , UDT    )

