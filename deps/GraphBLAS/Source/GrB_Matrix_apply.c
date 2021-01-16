//------------------------------------------------------------------------------
// GrB_Matrix_apply: apply a unary or binary operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_apply.h"
#include "GB_scalar.h"

//------------------------------------------------------------------------------
// GrB_Matrix_apply: apply a unary operator to a matrix
//------------------------------------------------------------------------------

// C<M> = accum(C,op(A)) or accum(C,op(A'))

GrB_Info GrB_Matrix_apply           // C<M> = accum (C, op(A)) or op(A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for C, M, and A
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE (C, "GrB_Matrix_apply (C, M, accum, op, A, desc)") ;
    GB_BURBLE_START ("GrB_apply") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        A_transpose, xx1, xx2, xx7) ;

    //--------------------------------------------------------------------------
    // apply the operator and optionally transpose
    //--------------------------------------------------------------------------

    info = GB_apply (
        C, C_replace,               // C and its descriptor
        M, Mask_comp, Mask_struct,  // mask and its descriptor
        accum,                      // optional accum for Z=accum(C,T)
        op,                         // operator op(.) to apply to the entries
        NULL, NULL, false,          // no binary operator
        A, A_transpose,             // A and its descriptor
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GB_1st: apply a binary operator: op(x,A)
//------------------------------------------------------------------------------

static inline GrB_Info GB_1st       // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GxB_Scalar x,             // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc,      // descriptor for C, M, and A
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_BURBLE_START ("GrB_apply") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (x) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        A_transpose, xx1, xx2, xx7) ;

    //--------------------------------------------------------------------------
    // apply the operator and optionally transpose
    //--------------------------------------------------------------------------

    info = GB_apply (
        C, C_replace,               // C and its descriptor
        M, Mask_comp, Mask_struct,  // mask and its descriptor
        accum,                      // optional accum for Z=accum(C,T)
        NULL,                       // no unary operator
        op, x, true,                // operator op(x,.) to apply to the entries
        A, A_transpose,             // A and its descriptor
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GB_2nd: apply a binary operator: op(A,y)
//------------------------------------------------------------------------------

// C<M> = accum(C,op(A,y)) or accum(C,op(A,y'))

static inline GrB_Info GB_2nd       // C<M>=accum(C,op(A,y))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    const GxB_Scalar y,             // second input: scalar y
    const GrB_Descriptor desc,      // descriptor for C, M, and A
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_BURBLE_START ("GrB_apply") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (y) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        xx1, A_transpose, xx2, xx7) ;

    //--------------------------------------------------------------------------
    // apply the operator and optionally transpose
    //--------------------------------------------------------------------------

    info = GB_apply (
        C, C_replace,               // C and its descriptor
        M, Mask_comp, Mask_struct,  // mask and its descriptor
        accum,                      // optional accum for Z=accum(C,T)
        NULL,                       // no unary operator
        op, y, false,               // operator op(.,y) to apply to the entries
        A, A_transpose,             // A and its descriptor
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GxB_Matrix_apply_BinaryOp1st: apply a binary operator: op(x,A)
//------------------------------------------------------------------------------

// C<M> = accum(C,op(x,A)) or accum(C,op(x,A'))

GrB_Info GxB_Matrix_apply_BinaryOp1st           // C<M>=accum(C,op(x,A))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GxB_Scalar x,             // first input:  scalar x
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for C, M, and A
)
{ 
    GB_WHERE (C, "GxB_Matrix_apply_BinaryOp1st (C, M, accum, op, x, A, desc)") ;
    return (GB_1st (C, M, accum, op, x, A, desc, Context)) ;
}

//------------------------------------------------------------------------------
// GxB_Matrix_apply_BinaryOp2nd: apply a binary operator: op(A,y)
//------------------------------------------------------------------------------

// C<M> = accum(C,op(A,y)) or accum(C,op(A,y'))

GrB_Info GxB_Matrix_apply_BinaryOp2nd           // C<M>=accum(C,op(A,y))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    const GxB_Scalar y,             // second input: scalar y
    const GrB_Descriptor desc       // descriptor for C, M, and A
)
{ 
    GB_WHERE (C, "GxB_Matrix_apply_BinaryOp2nd (C, M, accum, op, A, y, desc)") ;
    return (GB_2nd (C, M, accum, op, A, y, desc, Context)) ;
}

//------------------------------------------------------------------------------
// GrB_Matrix_apply_BinaryOp1st_TYPE: apply a binary operator: op(x,A)
//------------------------------------------------------------------------------

#define GB_BIND1ST(prefix,type,T,ampersand,stype)                           \
GrB_Info prefix ## Matrix_apply_BinaryOp1st_ ## T                           \
(                                                                           \
    GrB_Matrix C,                   /* input/output matrix for results */   \
    const GrB_Matrix M,             /* optional mask for C*/                \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(C,T) */   \
    const GrB_BinaryOp op,          /* operator to apply to the entries */  \
    const type x,                   /* first input:  scalar x */            \
    const GrB_Matrix A,             /* second input: matrix A */            \
    const GrB_Descriptor desc       /* descriptor for C, M, and A */        \
)                                                                           \
{                                                                           \
    GB_WHERE (C, GB_STR(prefix) "Matrix_apply_BinaryOp1st_" GB_STR(T)       \
        "(C, M, accum, op, x, A, desc)") ;                                  \
    GB_SCALAR_WRAP (scalar, prefix, T, ampersand, x, stype) ;               \
    ASSERT_SCALAR_OK (scalar, "scalar for matrix_apply_bind1st", GB0) ;     \
    return (GB_1st (C, M, accum, op, scalar, A, desc, Context)) ;           \
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
// GrB_Matrix_apply_BinaryOp2nd_TYPE: apply a binary operator: op(A,y)
//------------------------------------------------------------------------------

#define GB_BIND2ND(prefix,type,T,ampersand,stype)                           \
GrB_Info prefix ## Matrix_apply_BinaryOp2nd_ ## T                           \
(                                                                           \
    GrB_Matrix C,                   /* input/output matrix for results */   \
    const GrB_Matrix M,             /* optional mask for C*/                \
    const GrB_BinaryOp accum,       /* optional accum for Z=accum(C,T) */   \
    const GrB_BinaryOp op,          /* operator to apply to the entries */  \
    const GrB_Matrix A,             /* first input:  matrix A */            \
    const type y,                   /* second input: scalar y */            \
    const GrB_Descriptor desc       /* descriptor for C, M, and A */        \
)                                                                           \
{                                                                           \
    GB_WHERE (C, GB_STR(prefix) "Matrix_apply_BinaryOp2nd_" GB_STR(T)       \
        "(C, M, accum, op, A, y, desc)") ;                                  \
    GB_SCALAR_WRAP (scalar, prefix, T, ampersand, y, stype) ;               \
    ASSERT_SCALAR_OK (scalar, "scalar for matrix_apply_bind2nd", GB0) ;     \
    return (GB_2nd (C, M, accum, op, A, scalar, desc, Context)) ;           \
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

