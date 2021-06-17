//------------------------------------------------------------------------------
// GrB_Vector_apply: apply a unary or binary operator to a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_apply.h"
#include "GB_scalar.h"

//------------------------------------------------------------------------------
// GrB_Vector_apply: apply a unary operator to a vector
//------------------------------------------------------------------------------

GrB_Info GrB_Vector_apply           // w<M> = accum (w, op(u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE (w, "GrB_Vector_apply (w, M, accum, op, u, desc)") ;
    GB_BURBLE_START ("GrB_apply") ;
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;

    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (M == NULL || GB_VECTOR_OK (M)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        xx1, xx2, xx3, xx7) ;

    //--------------------------------------------------------------------------
    // apply the operator; do not transpose
    //--------------------------------------------------------------------------

    info = GB_apply (
        (GrB_Matrix) w, C_replace,  // w and its descriptor
        (GrB_Matrix) M, Mask_comp, Mask_struct, // mask and its descriptor
        accum,                      // optional accum for Z=accum(w,T)
        op,                         // operator op(.) to apply to the entries
        NULL, NULL, false,          // no binary operator
        (GrB_Matrix) u, false,      // u, not transposed
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GB_1st: apply a binary operator: op(x,u)
//------------------------------------------------------------------------------

static inline GrB_Info GB_1st       // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GxB_Scalar x,             // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc,      // descriptor for w and M
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_BURBLE_START ("GrB_apply") ;
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (x) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;

    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (M == NULL || GB_VECTOR_OK (M)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        xx1, xx2, xx3, xx7) ;

    //--------------------------------------------------------------------------
    // apply the operator; do not transpose
    //--------------------------------------------------------------------------

    info = GB_apply (
        (GrB_Matrix) w, C_replace,  // w and its descriptor
        (GrB_Matrix) M, Mask_comp, Mask_struct, // mask and its descriptor
        accum,                      // optional accum for Z=accum(w,T)
        NULL,                       // no unary operator
        op, x, true,                // operator op(x,.) to apply to the entries
        (GrB_Matrix) u,  false,     // u, not transposed
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GB_2nd: apply a binary operator: op(u,y)
//------------------------------------------------------------------------------

static inline GrB_Info GB_2nd       // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    const GxB_Scalar y,             // second input: scalar y
    const GrB_Descriptor desc,      // descriptor for w and M
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_BURBLE_START ("GrB_apply") ;
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;
    GB_RETURN_IF_NULL_OR_FAULTY (y) ;

    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (M == NULL || GB_VECTOR_OK (M)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        xx1, xx2, xx3, xx7) ;

    //--------------------------------------------------------------------------
    // apply the operator; do not transpose
    //--------------------------------------------------------------------------

    info = GB_apply (
        (GrB_Matrix) w, C_replace,  // w and its descriptor
        (GrB_Matrix) M, Mask_comp, Mask_struct, // mask and its descriptor
        accum,                      // optional accum for Z=accum(w,T)
        NULL,                       // no unary operator
        op, y, false,               // operator op(.,y) to apply to the entries
        (GrB_Matrix) u, false,      // u, not transposed
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GxB_Vector_apply_BinaryOp1st: apply a binary operator: op(x,u)
//------------------------------------------------------------------------------

GrB_Info GxB_Vector_apply_BinaryOp1st           // w<mask> = accum (w, op(x,u))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GxB_Scalar x,             // first input:  scalar x
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 
    GB_WHERE (w, "GxB_Vector_apply_BinaryOp1st (w, M, accum, op, x, u, desc)") ;
    return (GB_1st (w, M, accum, op, x, u, desc, Context)) ;
}

//------------------------------------------------------------------------------
// GxB_Vector_apply_BinaryOp2nd: apply a binary operator: op(u,y)
//------------------------------------------------------------------------------

GrB_Info GxB_Vector_apply_BinaryOp2nd           // w<mask> = accum (w, op(u,y))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Vector u,             // first input:  vector u
    const GxB_Scalar y,             // second input: scalar y
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 
    GB_WHERE (w, "GxB_Vector_apply_BinaryOp2nd (w, M, accum, op, u, y, desc)") ;
    return (GB_2nd (w, M, accum, op, u, y, desc, Context)) ;
}

//------------------------------------------------------------------------------
// GrB_Vector_apply_BinaryOp1st_TYPE: apply a binary operator: op(x,u)
//------------------------------------------------------------------------------

#define GB_BIND1ST(prefix,type,T,ampersand,stype)                           \
GrB_Info prefix ## Vector_apply_BinaryOp1st_ ## T                           \
(                                                                           \
    GrB_Vector w,                   /* input/output vector for results */   \
    const GrB_Vector M,             /* optional mask for w */               \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(w,T) */   \
    const GrB_BinaryOp op,          /* operator to apply to the entries */  \
    const type x,                   /* first input:  scalar x */            \
    const GrB_Vector u,             /* second input: vector u */            \
    const GrB_Descriptor desc       /* descriptor for w and M */            \
)                                                                           \
{                                                                           \
    GB_WHERE (w, GB_STR(prefix) "Vector_apply_BinaryOp1st_" GB_STR(T)       \
        "(w, M, accum, op, x, u, desc)") ;                                  \
    GB_SCALAR_WRAP (scalar, prefix, T, ampersand, x, stype) ;               \
    ASSERT_SCALAR_OK (scalar, "scalar for vector_apply_bind1st", GB0) ;     \
    return (GB_1st (w, M, accum, op, scalar, u, desc, Context)) ;           \
}

GB_BIND1ST (GrB_, bool      , BOOL   , &, GrB_BOOL  )
GB_BIND1ST (GrB_, int8_t    , INT8   , &, GrB_INT8  )
GB_BIND1ST (GrB_, int16_t   , INT16  , &, GrB_INT16 )
GB_BIND1ST (GrB_, int32_t   , INT32  , &, GrB_INT32 )
GB_BIND1ST (GrB_, int64_t   , INT64  , &, GrB_INT64 )
GB_BIND1ST (GrB_, uint8_t   , UINT8  , &, GrB_UINT8 )
GB_BIND1ST (GrB_, uint16_t  , UINT16 , &, GrB_UINT16)
GB_BIND1ST (GrB_, uint32_t  , UINT32 , &, GrB_UINT32)
GB_BIND1ST (GrB_, uint64_t  , UINT64 , &, GrB_UINT64)
GB_BIND1ST (GrB_, float     , FP32   , &, GrB_FP32  )
GB_BIND1ST (GrB_, double    , FP64   , &, GrB_FP64  )
GB_BIND1ST (GxB_, GxB_FC32_t, FC32   , &, GxB_FC32  )
GB_BIND1ST (GxB_, GxB_FC64_t, FC64   , &, GxB_FC64  )
GB_BIND1ST (GrB_, void *    , UDT    ,  , op->xtype )

//------------------------------------------------------------------------------
// GrB_Vector_apply_BinaryOp2nd_TYPE: apply a binary operator: op(u,y)
//------------------------------------------------------------------------------

#define GB_BIND2ND(prefix,type,T,ampersand,stype)                           \
GrB_Info prefix ## Vector_apply_BinaryOp2nd_ ## T                           \
(                                                                           \
    GrB_Vector w,                   /* input/output vector for results */   \
    const GrB_Vector M,             /* optional mask for w*/                \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(w,T) */   \
    const GrB_BinaryOp op,          /* operator to apply to the entries */  \
    const GrB_Vector u,             /* first input:  vector u */            \
    const type y,                   /* second input: scalar y */            \
    const GrB_Descriptor desc       /* descriptor for w and M */            \
)                                                                           \
{                                                                           \
    GB_WHERE (w, GB_STR(prefix) "Vector_apply_BinaryOp2nd_" GB_STR(T)       \
        "(w, M, accum, op, u, y, desc)") ;                                  \
    GB_SCALAR_WRAP (scalar, prefix, T, ampersand, y, stype) ;               \
    ASSERT_SCALAR_OK (scalar, "scalar for vector_apply_bind2nd", GB0) ;     \
    return (GB_2nd (w, M, accum, op, u, scalar, desc, Context)) ;           \
}

GB_BIND2ND (GrB_, bool      , BOOL   , &, GrB_BOOL  )
GB_BIND2ND (GrB_, int8_t    , INT8   , &, GrB_INT8  )
GB_BIND2ND (GrB_, int16_t   , INT16  , &, GrB_INT16 )
GB_BIND2ND (GrB_, int32_t   , INT32  , &, GrB_INT32 )
GB_BIND2ND (GrB_, int64_t   , INT64  , &, GrB_INT64 )
GB_BIND2ND (GrB_, uint8_t   , UINT8  , &, GrB_UINT8 )
GB_BIND2ND (GrB_, uint16_t  , UINT16 , &, GrB_UINT16)
GB_BIND2ND (GrB_, uint32_t  , UINT32 , &, GrB_UINT32)
GB_BIND2ND (GrB_, uint64_t  , UINT64 , &, GrB_UINT64)
GB_BIND2ND (GrB_, float     , FP32   , &, GrB_FP32  )
GB_BIND2ND (GrB_, double    , FP64   , &, GrB_FP64  )
GB_BIND2ND (GxB_, GxB_FC32_t, FC32   , &, GxB_FC32  )
GB_BIND2ND (GxB_, GxB_FC64_t, FC64   , &, GxB_FC64  )
GB_BIND2ND (GrB_, void *    , UDT    ,  , op->ytype )

