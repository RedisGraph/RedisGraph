//------------------------------------------------------------------------------
// GrB_Vector_build: build a sparse GraphBLAS vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

#define GB_BUILD(type,T)                                                      \
GrB_Info GrB_Vector_build_ ## T     /* build a vector from (I,X) tuples   */  \
(                                                                             \
    GrB_Vector w,                   /* vector to build                    */  \
    const GrB_Index *I,             /* array of row indices of tuples     */  \
    const type *X,                  /* array of values of tuples          */  \
    GrB_Index nvals,                /* number of tuples                   */  \
    const GrB_BinaryOp dup          /* binary op to assemble duplicates   */  \
)                                                                             \
{                                                                             \
    GB_WHERE ("GrB_Vector_build_" GB_STR(T) " (w, I, X, nvals, dup)") ;       \
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;                                         \
    ASSERT (GB_VECTOR_OK (w)) ;                                               \
    GrB_Info info = GB_user_build ((GrB_Matrix) w, I, NULL, X, nvals, dup,    \
        GB_ ## T ## _code, false, Context) ;                                  \
    ASSERT (GB_IMPLIES (info == GrB_SUCCESS, GB_VECTOR_OK (w))) ;             \
    return (info) ;                                                           \
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

