//------------------------------------------------------------------------------
// GrB_Matrix_reduce_scalar: reduce a matrix to a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reduce entries in a matrix to a scalar, c = accum (c, reduce_to_scalar(A)))

// All entries in the matrix are "summed" to a single scalar t using the reduce
// monoid, which must be associative (otherwise the results are undefined).
// The result is either assigned to the output scalar c (if accum is NULL), or
// it accumulated in the result c via c = accum(c,t).  If A has no entries, the
// result t is the identity value of the monoid.  Unlike most other GraphBLAS
// operations, this operation uses an accum operator but no mask.

#include "GB.h"

#define GB_REDUCE(type,T)                                                      \
GrB_Info GrB_Matrix_reduce_ ## T    /* c = accum (c, reduce_to_scalar (A))  */ \
(                                                                              \
    type *c,                        /* result scalar                        */ \
    const GrB_BinaryOp accum,       /* optional accum for c=accum(c,t)      */ \
    const GrB_Monoid reduce,        /* monoid to do the reduction           */ \
    const GrB_Matrix A,             /* matrix to reduce                     */ \
    const GrB_Descriptor desc       /* descriptor (currently unused)        */ \
)                                                                              \
{                                                                              \
    GB_WHERE ("GrB_Matrix_reduce_" GB_STR(T) " (&c, accum, reduce, A, desc)") ;\
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;                                          \
    return (GB_reduce_to_scalar (c, GrB_ ## T, accum, reduce, A, Context)) ;   \
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

GrB_Info GrB_Matrix_reduce_UDT      // c = accum (c, reduce_to_scalar (A))
(
    void *c,                        // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid reduce,        // monoid to do the reduction
    const GrB_Matrix A,             // matrix to reduce
    const GrB_Descriptor desc       // descriptor (currently unused)
)
{ 
    // Reduction to a user-defined type requires an assumption about the type
    // of the scalar c.  It's just a void* pointer so its type must be
    // inferred from the other arguments.  The type cannot be found from
    // accum, since accum can be NULL.  The result is computed by the reduce
    // monoid, and no typecasting can be done between user-defined types.
    // Thus, the type of c must be the same as the reduce monoid.

    GB_WHERE ("GrB_Matrix_reduce_UDT (&c, accum, reduce, A, desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (reduce) ;
    return (GB_reduce_to_scalar (c, reduce->op->ztype, accum, reduce, A,
        Context)) ;
}

