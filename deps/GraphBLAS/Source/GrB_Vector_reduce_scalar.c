//------------------------------------------------------------------------------
// GrB_Vector_reduce_scalar: reduce a vector to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reduce entries in a vector to a scalar, c = accum (c, reduce_to_scalar(u))

// All entries in the vector are "summed" to a single scalar t using the reduce
// monoid, which must be associative (otherwise the results are undefined).
// The result is either assigned to the output scalar c (if accum is NULL), or
// it accumulated in the result c via c = accum(c,t).  If the u has no entries,
// the result t is the identity value of the monoid.  Unlike most other
// GraphBLAS operations, this operation uses an accum operator but no mask.

#include "GB.h"

#define GB_REDUCE(type,T)                                                      \
GrB_Info GrB_Vector_reduce_ ## T    /* c = accum (c, reduce_to_scalar (u))  */ \
(                                                                              \
    type *c,                        /* result scalar                        */ \
    const GrB_BinaryOp accum,       /* optional accum for c=accum(c,t)      */ \
    const GrB_Monoid reduce,        /* monoid to do the reduction           */ \
    const GrB_Vector u,             /* vector to reduce                     */ \
    const GrB_Descriptor desc       /* descriptor (currently unused)        */ \
)                                                                              \
{                                                                              \
    GB_WHERE ("GrB_Vector_reduce_" GB_STR(T) " (&c, accum, reduce, u, desc)") ;\
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;                                          \
    ASSERT (GB_VECTOR_OK (u)) ;                                                \
    return (GB_reduce_to_scalar (c, GrB_ ## T, accum, reduce, (GrB_Matrix) u,  \
        Context)) ;                                                     \
}

GB_REDUCE (bool     , BOOL   )
GB_REDUCE (int8_t   , INT8   )
GB_REDUCE (uint8_t  , UINT8  )
GB_REDUCE (int16_t  , INT16  )
GB_REDUCE (uint16_t , UINT16 )
GB_REDUCE (int32_t  , INT32  )
GB_REDUCE (uint32_t , UINT32 )
GB_REDUCE (int64_t  , INT64  )
GB_REDUCE (uint64_t , UINT64 )
GB_REDUCE (float    , FP32   )
GB_REDUCE (double   , FP64   )

GrB_Info GrB_Vector_reduce_UDT      // c = accum (c, reduce_to_scalar (u))
(
    void *c,                        // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid reduce,        // monoid to do the reduction
    const GrB_Vector u,             // vector to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
)
{ 
    // See comments on GrB_Matrix_reduce_UDT
    GB_WHERE ("GrB_Vector_reduce_UDT (&c, accum, reduce, u, desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;
    GB_RETURN_IF_NULL_OR_FAULTY (reduce) ;
    ASSERT (GB_VECTOR_OK (u)) ;
    return (GB_reduce_to_scalar (c, reduce->op->ztype,
        accum, reduce, (GrB_Matrix) u, Context)) ;
}

