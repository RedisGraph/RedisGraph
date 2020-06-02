//------------------------------------------------------------------------------
// GrB_Matrix_extractElement: extract a single entry from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Extract the value of single scalar, x = A(row,col), typecasting from the
// type of A to the type of x, as needed.

// Returns GrB_SUCCESS if A(row,col) is present, and sets x to its value.
// Returns GrB_NO_VALUE if A(row,col) is not present, and x is unmodified.

#include "GB.h"

#define GB_EXTRACT(type,T)                                                    \
GrB_Info GrB_Matrix_extractElement_ ## T     /* x = A(row,col) */             \
(                                                                             \
    type *x,                            /* extracted scalar                */ \
    const GrB_Matrix A,                 /* matrix to extract a scalar from */ \
    GrB_Index row,                      /* row index                       */ \
    GrB_Index col                       /* column index                    */ \
)                                                                             \
{                                                                             \
    GB_WHERE ("GrB_Matrix_extractElement_" GB_STR(T) " (x, A, row, col)") ;   \
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;                                         \
    return (GB_extractElement (x, GB_ ## T ## _code, A, row, col, Context)) ; \
}

GB_EXTRACT (bool     , BOOL   )
GB_EXTRACT (int8_t   , INT8   )
GB_EXTRACT (uint8_t  , UINT8  )
GB_EXTRACT (int16_t  , INT16  )
GB_EXTRACT (uint16_t , UINT16 )
GB_EXTRACT (int32_t  , INT32  )
GB_EXTRACT (uint32_t , UINT32 )
GB_EXTRACT (int64_t  , INT64  )
GB_EXTRACT (uint64_t , UINT64 )
GB_EXTRACT (float    , FP32   )
GB_EXTRACT (double   , FP64   )
GB_EXTRACT (void     , UDT    )

