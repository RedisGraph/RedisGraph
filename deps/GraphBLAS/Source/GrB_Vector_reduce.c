//------------------------------------------------------------------------------
// GrB_Vector_reduce: reduce a vector to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Reduce entries in a vector to a scalar, c = accum (c, reduce_to_scalar(u))

// All entries in the vector are "summed" to a single scalar t using the reduce
// monoid, which must be associative (otherwise the results are undefined).
// The result is either assigned to the output scalar c (if accum is NULL), or
// it accumulated in the result c via c = accum(c,t).  If the u has no entries,
// the result t is the identity value of the monoid.  Unlike most other
// GraphBLAS operations, this operation uses an accum operator but no mask.

#include "GB_reduce.h"

#define GB_VECTOR_TO_SCALAR(prefix,type,T)                                    \
GrB_Info GB_EVAL3 (prefix, _Vector_reduce_, T) /* c = accum (c, reduce (u))*/ \
(                                                                             \
    type *c,                        /* result scalar                       */ \
    const GrB_BinaryOp accum,       /* optional accum for c=accum(c,t)     */ \
    const GrB_Monoid monoid,        /* monoid to do the reduction          */ \
    const GrB_Vector u,             /* vector to reduce                    */ \
    const GrB_Descriptor desc       /* descriptor (currently unused)       */ \
)                                                                             \
{                                                                             \
    GB_WHERE1 ("GrB_Vector_reduce_" GB_STR(T)                                 \
        " (&c, accum, monoid, u, desc)") ;                                    \
    GB_BURBLE_START ("GrB_reduce") ;                                          \
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;                                         \
    ASSERT (GB_VECTOR_OK (u)) ;                                               \
    GrB_Info info = GB_reduce_to_scalar (c, GB_EVAL3 (prefix, _, T), accum,   \
        monoid, (GrB_Matrix) u, Context) ;                                    \
    GB_BURBLE_END ;                                                           \
    return (info) ;                                                           \
}

GB_VECTOR_TO_SCALAR (GrB, bool      , BOOL   )
GB_VECTOR_TO_SCALAR (GrB, int8_t    , INT8   )
GB_VECTOR_TO_SCALAR (GrB, int16_t   , INT16  )
GB_VECTOR_TO_SCALAR (GrB, int32_t   , INT32  )
GB_VECTOR_TO_SCALAR (GrB, int64_t   , INT64  )
GB_VECTOR_TO_SCALAR (GrB, uint8_t   , UINT8  )
GB_VECTOR_TO_SCALAR (GrB, uint16_t  , UINT16 )
GB_VECTOR_TO_SCALAR (GrB, uint32_t  , UINT32 )
GB_VECTOR_TO_SCALAR (GrB, uint64_t  , UINT64 )
GB_VECTOR_TO_SCALAR (GrB, float     , FP32   )
GB_VECTOR_TO_SCALAR (GrB, double    , FP64   )
GB_VECTOR_TO_SCALAR (GxB, GxB_FC32_t, FC32   )
GB_VECTOR_TO_SCALAR (GxB, GxB_FC64_t, FC64   )

GrB_Info GrB_Vector_reduce_UDT      // c = accum (c, reduce_to_scalar (u))
(
    void *c,                        // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
)
{ 
    // See comments on GrB_Matrix_reduce_UDT
    GB_WHERE1 ("GrB_Vector_reduce_UDT (&c, accum, monoid, u, desc)") ;
    GB_BURBLE_START ("GrB_reduce") ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;
    ASSERT (GB_VECTOR_OK (u)) ;
    GrB_Info info = GB_reduce_to_scalar (c, monoid->op->ztype,
        accum, monoid, (GrB_Matrix) u, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}

