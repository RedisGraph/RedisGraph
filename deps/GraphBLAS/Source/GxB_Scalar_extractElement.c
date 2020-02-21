//------------------------------------------------------------------------------
// GxB_Scalar_extractElement: extract a single entry from a GxB_Scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Extract a single entry, x = s, typecasting from the type
// of s to the type of x, as needed.

// Returns GrB_SUCCESS if s is present, and sets x to its value.
// Returns GrB_NO_VALUE if s does not have an entry, and x is unmodified.

#include "GB.h"

#define GB_EXTRACT(type,T)                                                    \
GrB_Info GxB_Scalar_extractElement_ ## T     /* x = s */                      \
(                                                                             \
    type *x,                /* user scalar extracted */                       \
    const GxB_Scalar s      /* GxB_Scalar to extract an entry from */         \
)                                                                             \
{                                                                             \
    GB_WHERE ("GxB_Scalar_extractElement_" GB_STR(T) " (x, s)") ;             \
    GB_RETURN_IF_NULL_OR_FAULTY (s) ;                                         \
    ASSERT (GB_SCALAR_OK (s)) ;                                               \
    return (GB_extractElement (x, GB_ ## T ## _code, (GrB_Matrix) s, 0, 0,    \
        Context)) ;                                                           \
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

