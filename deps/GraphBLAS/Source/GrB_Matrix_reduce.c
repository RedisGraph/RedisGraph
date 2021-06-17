//------------------------------------------------------------------------------
// GrB_Matrix_reduce: reduce a matrix to a vector or scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------
// GrB_Matrix_reduce_TYPE: reduce a matrix to a scalar
//------------------------------------------------------------------------------

// Reduce entries in a matrix to a scalar, c = accum (c, reduce_to_scalar(A)))

// All entries in the matrix are "summed" to a single scalar t using the reduce
// monoid.  The result is either assigned to the output scalar c (if accum is
// NULL), or it accumulated in the result c via c = accum(c,t).  If A has no
// entries, the result t is the identity value of the monoid.  Unlike most
// other GraphBLAS operations, this operation uses an accum operator but no
// mask.

#include "GB_reduce.h"
#include "GB_binop.h"

#define GB_MATRIX_TO_SCALAR(prefix,type,T)                                     \
GrB_Info prefix ## Matrix_reduce_ ## T    /* c = accum (c, reduce (A))  */     \
(                                                                              \
    type *c,                        /* result scalar                        */ \
    const GrB_BinaryOp accum,       /* optional accum for c=accum(c,t)      */ \
    const GrB_Monoid monoid,        /* monoid to do the reduction           */ \
    const GrB_Matrix A,             /* matrix to reduce                     */ \
    const GrB_Descriptor desc       /* descriptor (currently unused)        */ \
)                                                                              \
{                                                                              \
    GB_WHERE1 ("Matrix_reduce_" GB_STR(T) " (&c, accum, monoid, A, desc)") ;   \
    GB_BURBLE_START ("GrB_reduce") ;                                           \
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;                                          \
    GrB_Info info = GB_reduce_to_scalar (c, prefix ## T, accum, monoid,        \
        A, Context) ;                                                          \
    GB_BURBLE_END ;                                                            \
    return (info) ;                                                            \
}

GB_MATRIX_TO_SCALAR (GrB_, bool      , BOOL   )
GB_MATRIX_TO_SCALAR (GrB_, int8_t    , INT8   )
GB_MATRIX_TO_SCALAR (GrB_, int16_t   , INT16  )
GB_MATRIX_TO_SCALAR (GrB_, int32_t   , INT32  )
GB_MATRIX_TO_SCALAR (GrB_, int64_t   , INT64  )
GB_MATRIX_TO_SCALAR (GrB_, uint8_t   , UINT8  )
GB_MATRIX_TO_SCALAR (GrB_, uint16_t  , UINT16 )
GB_MATRIX_TO_SCALAR (GrB_, uint32_t  , UINT32 )
GB_MATRIX_TO_SCALAR (GrB_, uint64_t  , UINT64 )
GB_MATRIX_TO_SCALAR (GrB_, float     , FP32   )
GB_MATRIX_TO_SCALAR (GrB_, double    , FP64   )
GB_MATRIX_TO_SCALAR (GxB_, GxB_FC32_t, FC32   )
GB_MATRIX_TO_SCALAR (GxB_, GxB_FC64_t, FC64   )

GrB_Info GrB_Matrix_reduce_UDT      // c = accum (c, reduce_to_scalar (A))
(
    void *c,                        // result scalar
    const GrB_BinaryOp accum,       // optional accum for c=accum(c,t)
    const GrB_Monoid monoid,        // monoid to do the reduction
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

    GB_WHERE1 ("GrB_Matrix_reduce_UDT (&c, accum, monoid, A, desc)") ;
    GB_BURBLE_START ("GrB_reduce") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;
    GrB_Info info = GB_reduce_to_scalar (c, monoid->op->ztype, accum, monoid,
        A, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GrB_Matrix_reduce_Monoid: reduce a matrix to a vector via a monoid
//------------------------------------------------------------------------------

GrB_Info GrB_Matrix_reduce_Monoid   // w<M> = accum (w,reduce(A))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Monoid monoid,        // reduce monoid for t=reduce(A)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for w, M, and A
)
{ 
    GB_WHERE (w, "GrB_Matrix_reduce_Monoid (w, M, accum, monoid, A, desc)") ;
    GB_BURBLE_START ("GrB_reduce") ;
    GrB_Info info = GB_reduce_to_vector ((GrB_Matrix) w, (GrB_Matrix) M,
        accum, monoid, A, desc, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GrB_Matrix_reduce_BinaryOp: reduce a matrix to a vector via a binary op
//------------------------------------------------------------------------------

// In my opinion, GrB_Matrix_reduce_BinaryOp should not be in the GraphBLAS C
// API Specification.  All other case of reduction in the C API, including all
// other uses of GrB_reduce, GrB_vxm, GrB_mxv, and GrB_mxm, require a monoid
// with an identity value.  This function is an outlier.  Obtaining good
// performance in a parallel implementation requires knowledge of the monoid
// identity value.  To implement this method in SuiteSparse:GraphBLAS, the
// binary operator "op_in" is promoted to the corresponding monoid, for
// built-in operators:
//
//      operator                data-types (all built-in)
//      ----------------------  ---------------------------
//      MIN, MAX                int*, uint*, fp*
//      TIMES, PLUS             int*, uint*, fp*, fc*
//      ANY                     int*, uint*, fp*, fc*, bool
//      LOR, LAND, LXOR, EQ     bool
//      BOR, BAND, BXOR, BXNOR  uint*
//
// No other cases are supported.  In particular, user-defined types and
// operators are not supported, since in those cases the identity value cannot
// be inferred.  Use GrB_Matrix_reduce_Monoid with a user-defined monoid
// instead.  The use of this function in SuiteSparse:GraphBLAS is discouraged.

GrB_Info GrB_Matrix_reduce_BinaryOp
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op_in,       // reduce operator for t=reduce(A)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for w, M, and A
)
{ 
    GB_WHERE (w, "GrB_Matrix_reduce_BinaryOp (w, M, accum, op, A, desc)") ;
    GB_BURBLE_START ("GrB_reduce") ;

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL_OR_FAULTY (op_in) ;
    ASSERT_BINARYOP_OK (op_in, "binary op for reduce-to-vector", GB0) ;

    // check operator types; all must be identical
    if (op_in->xtype != op_in->ztype || op_in->ytype != op_in->ztype)
    { 
        GB_ERROR (GrB_DOMAIN_MISMATCH, "Invalid binary operator:"
            " z=%s(x,y) has no equivalent monoid\n", op_in->name) ;
    }

    //--------------------------------------------------------------------------
    // convert the binary op_in to its corresponding monoid
    //--------------------------------------------------------------------------

    GrB_Monoid monoid = NULL ;
    GrB_BinaryOp op = GB_boolean_rename_op (op_in) ;
    GB_Type_code zcode = op->ztype->code ;
    GB_Opcode opcode = op->opcode ;

    switch (opcode)
    {

        case GB_MIN_opcode:

            switch (zcode)
            {
                // 10 MIN monoids: for 10 real types
                case GB_INT8_code   : monoid = GrB_MIN_MONOID_INT8     ; break ;
                case GB_INT16_code  : monoid = GrB_MIN_MONOID_INT16    ; break ;
                case GB_INT32_code  : monoid = GrB_MIN_MONOID_INT32    ; break ;
                case GB_INT64_code  : monoid = GrB_MIN_MONOID_INT64    ; break ;
                case GB_UINT8_code  : monoid = GrB_MIN_MONOID_UINT8    ; break ;
                case GB_UINT16_code : monoid = GrB_MIN_MONOID_UINT16   ; break ;
                case GB_UINT32_code : monoid = GrB_MIN_MONOID_UINT32   ; break ;
                case GB_UINT64_code : monoid = GrB_MIN_MONOID_UINT64   ; break ;
                case GB_FP32_code   : monoid = GrB_MIN_MONOID_FP32     ; break ;
                case GB_FP64_code   : monoid = GrB_MIN_MONOID_FP64     ; break ;
                default: ;
            }
            break ;

        case GB_MAX_opcode:

            switch (zcode)
            {
                // 10 MAX monoids: for 10 real types
                case GB_INT8_code   : monoid = GrB_MAX_MONOID_INT8     ; break ;
                case GB_INT16_code  : monoid = GrB_MAX_MONOID_INT16    ; break ;
                case GB_INT32_code  : monoid = GrB_MAX_MONOID_INT32    ; break ;
                case GB_INT64_code  : monoid = GrB_MAX_MONOID_INT64    ; break ;
                case GB_UINT8_code  : monoid = GrB_MAX_MONOID_UINT8    ; break ;
                case GB_UINT16_code : monoid = GrB_MAX_MONOID_UINT16   ; break ;
                case GB_UINT32_code : monoid = GrB_MAX_MONOID_UINT32   ; break ;
                case GB_UINT64_code : monoid = GrB_MAX_MONOID_UINT64   ; break ;
                case GB_FP32_code   : monoid = GrB_MAX_MONOID_FP32     ; break ;
                case GB_FP64_code   : monoid = GrB_MAX_MONOID_FP64     ; break ;
                default: ;
            }
            break ;

        case GB_TIMES_opcode:

            switch (zcode)
            {
                // 12 TIMES monoids: 10 real types, and 2 complex types
                case GB_INT8_code   : monoid = GrB_TIMES_MONOID_INT8   ; break ;
                case GB_INT16_code  : monoid = GrB_TIMES_MONOID_INT16  ; break ;
                case GB_INT32_code  : monoid = GrB_TIMES_MONOID_INT32  ; break ;
                case GB_INT64_code  : monoid = GrB_TIMES_MONOID_INT64  ; break ;
                case GB_UINT8_code  : monoid = GrB_TIMES_MONOID_UINT8  ; break ;
                case GB_UINT16_code : monoid = GrB_TIMES_MONOID_UINT16 ; break ;
                case GB_UINT32_code : monoid = GrB_TIMES_MONOID_UINT32 ; break ;
                case GB_UINT64_code : monoid = GrB_TIMES_MONOID_UINT64 ; break ;
                case GB_FP32_code   : monoid = GrB_TIMES_MONOID_FP32   ; break ;
                case GB_FP64_code   : monoid = GrB_TIMES_MONOID_FP64   ; break ;
                case GB_FC32_code   : monoid = GxB_TIMES_FC32_MONOID   ; break ;
                case GB_FC64_code   : monoid = GxB_TIMES_FC64_MONOID   ; break ;
                default: ;
            }
            break ;

        case GB_PLUS_opcode:

            switch (zcode)
            {
                // 12 PLUS monoids: 10 real types, and 2 complex types
                case GB_INT8_code   : monoid = GrB_PLUS_MONOID_INT8    ; break ;
                case GB_INT16_code  : monoid = GrB_PLUS_MONOID_INT16   ; break ;
                case GB_INT32_code  : monoid = GrB_PLUS_MONOID_INT32   ; break ;
                case GB_INT64_code  : monoid = GrB_PLUS_MONOID_INT64   ; break ;
                case GB_UINT8_code  : monoid = GrB_PLUS_MONOID_UINT8   ; break ;
                case GB_UINT16_code : monoid = GrB_PLUS_MONOID_UINT16  ; break ;
                case GB_UINT32_code : monoid = GrB_PLUS_MONOID_UINT32  ; break ;
                case GB_UINT64_code : monoid = GrB_PLUS_MONOID_UINT64  ; break ;
                case GB_FP32_code   : monoid = GrB_PLUS_MONOID_FP32    ; break ;
                case GB_FP64_code   : monoid = GrB_PLUS_MONOID_FP64    ; break ;
                case GB_FC32_code   : monoid = GxB_PLUS_FC32_MONOID    ; break ;
                case GB_FC64_code   : monoid = GxB_PLUS_FC64_MONOID    ; break ;
                default: ;
            }
            break ;

        case GB_ANY_opcode:

            switch (zcode)
            {
                // 13 ANY monoids: bool, 10 real types, and 2 complex types
                case GB_BOOL_code   : monoid = GxB_ANY_BOOL_MONOID     ; break ;
                case GB_INT8_code   : monoid = GxB_ANY_INT8_MONOID     ; break ;
                case GB_INT16_code  : monoid = GxB_ANY_INT16_MONOID    ; break ;
                case GB_INT32_code  : monoid = GxB_ANY_INT32_MONOID    ; break ;
                case GB_INT64_code  : monoid = GxB_ANY_INT64_MONOID    ; break ;
                case GB_UINT8_code  : monoid = GxB_ANY_UINT8_MONOID    ; break ;
                case GB_UINT16_code : monoid = GxB_ANY_UINT16_MONOID   ; break ;
                case GB_UINT32_code : monoid = GxB_ANY_UINT32_MONOID   ; break ;
                case GB_UINT64_code : monoid = GxB_ANY_UINT64_MONOID   ; break ;
                case GB_FP32_code   : monoid = GxB_ANY_FP32_MONOID     ; break ;
                case GB_FP64_code   : monoid = GxB_ANY_FP64_MONOID     ; break ;
                case GB_FC32_code   : monoid = GxB_ANY_FC32_MONOID     ; break ;
                case GB_FC64_code   : monoid = GxB_ANY_FC64_MONOID     ; break ;
                default: ;
            }
            break ;

        // 4 boolean monoids: (see also GxB_ANY_BOOL_MONOID above)
        #define B(x) if (zcode == GB_BOOL_code) monoid = x ; break ;
        case GB_LOR_opcode   : B (GrB_LOR_MONOID_BOOL)   ;
        case GB_LAND_opcode  : B (GrB_LAND_MONOID_BOOL)  ;
        case GB_LXOR_opcode  : B (GrB_LXOR_MONOID_BOOL)  ;
        case GB_EQ_opcode    : B (GrB_LXNOR_MONOID_BOOL) ;

        case GB_BOR_opcode:

            switch (zcode)
            {
                // 4 BOR monoids
                case GB_UINT8_code  : monoid = GxB_BOR_UINT8_MONOID    ; break ;
                case GB_UINT16_code : monoid = GxB_BOR_UINT16_MONOID   ; break ;
                case GB_UINT32_code : monoid = GxB_BOR_UINT32_MONOID   ; break ;
                case GB_UINT64_code : monoid = GxB_BOR_UINT64_MONOID   ; break ;
                default: ;
            }
            break ;

        case GB_BAND_opcode:

            switch (zcode)
            {
                // 4 BAND monoids
                case GB_UINT8_code  : monoid = GxB_BAND_UINT8_MONOID   ; break ;
                case GB_UINT16_code : monoid = GxB_BAND_UINT16_MONOID  ; break ;
                case GB_UINT32_code : monoid = GxB_BAND_UINT32_MONOID  ; break ;
                case GB_UINT64_code : monoid = GxB_BAND_UINT64_MONOID  ; break ;
                default: ;
            }
            break ;

        case GB_BXOR_opcode:

            switch (zcode)
            {
                // 4 BXOR monoids
                case GB_UINT8_code  : monoid = GxB_BXOR_UINT8_MONOID   ; break ;
                case GB_UINT16_code : monoid = GxB_BXOR_UINT16_MONOID  ; break ;
                case GB_UINT32_code : monoid = GxB_BXOR_UINT32_MONOID  ; break ;
                case GB_UINT64_code : monoid = GxB_BXOR_UINT64_MONOID  ; break ;
                default: ;
            }
            break ;

        case GB_BXNOR_opcode:

            switch (zcode)
            {
                // 4 BXNOR monoids
                case GB_UINT8_code  : monoid = GxB_BXNOR_UINT8_MONOID  ; break ;
                case GB_UINT16_code : monoid = GxB_BXNOR_UINT16_MONOID ; break ;
                case GB_UINT32_code : monoid = GxB_BXNOR_UINT32_MONOID ; break ;
                case GB_UINT64_code : monoid = GxB_BXNOR_UINT64_MONOID ; break ;
                default: ;
            }
            break ;

        default : 

            // op_in binary operator does not correspond to a known monoid
            GB_ERROR (GrB_DOMAIN_MISMATCH, "Invalid binary operator:"
                " z=%s(x,y) has no equivalent monoid\n", op_in->name) ;
    }

    //--------------------------------------------------------------------------
    // w<M> = reduce (A) via the monoid
    //--------------------------------------------------------------------------

    GrB_Info info = GB_reduce_to_vector ((GrB_Matrix) w, (GrB_Matrix) M,
        accum, monoid, A, desc, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}

