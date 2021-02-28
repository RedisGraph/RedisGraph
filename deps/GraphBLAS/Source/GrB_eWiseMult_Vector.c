//------------------------------------------------------------------------------
// GrB_eWiseMult_Vector: vector element-wise multiplication
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w<M> = accum (w,u.*v)

#include "GB_ewise.h"

#define GB_EWISE(op)                                                        \
    /* check inputs */                                                      \
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;                                       \
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;                                       \
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;                                       \
    GB_RETURN_IF_FAULTY (M) ;                                               \
    ASSERT (GB_VECTOR_OK (w)) ;                                             \
    ASSERT (GB_VECTOR_OK (u)) ;                                             \
    ASSERT (GB_VECTOR_OK (v)) ;                                             \
    ASSERT (M == NULL || GB_VECTOR_OK (M)) ;                                \
    /* get the descriptor */                                                \
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,       \
        xx1, xx2, xx3) ;                                                    \
    /* C<M> = accum (C,T) where T = A.*B, A'.*B, A.*B', or A'.*B' */        \
    info = GB_ewise (                                                       \
        (GrB_Matrix) w, C_replace,  /* w and its descriptor        */       \
        (GrB_Matrix) M, Mask_comp, Mask_struct,  /* mask and descriptor */  \
        accum,                      /* accumulate operator         */       \
        op,                         /* operator that defines '.*'  */       \
        (GrB_Matrix) u, false,      /* u, never transposed         */       \
        (GrB_Matrix) v, false,      /* v, never transposed         */       \
        false,                      /* eWiseMult                   */       \
        Context) ;

//------------------------------------------------------------------------------
// GrB_eWiseMult_Vector_BinaryOp: vector element-wise multiplication
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseMult_Vector_BinaryOp       // w<M> = accum (w, u.*v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp mult,        // defines '.*' for t=u.*v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseMult_Vector_BinaryOp (w, M, accum, mult, u, v, desc)") ;
    GB_BURBLE_START ("GrB_eWiseMult") ;
    GB_RETURN_IF_NULL_OR_FAULTY (mult) ;

    //--------------------------------------------------------------------------
    // apply the eWise kernel (using set intersection)
    //--------------------------------------------------------------------------

    GB_EWISE (mult) ;
    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseMult_Vector_Monoid: vector element-wise multiplication
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseMult_Vector_Monoid         // w<M> = accum (w, u.*v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Monoid monoid,        // defines '.*' for t=u.*v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseMult_Vector_Monoid (w, M, accum, monoid, u, v, desc)") ;
    GB_BURBLE_START ("GrB_eWiseMult") ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;

    //--------------------------------------------------------------------------
    // eWise multiply using the monoid operator
    //--------------------------------------------------------------------------

    GB_EWISE (monoid->op) ;
    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseMult_Vector_Semiring: vector element-wise multiplication
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseMult_Vector_Semiring       // w<M> = accum (w, u.*v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '.*' for t=u.*v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseMult_Vector_Semiring (w, M, accum, semiring, u, v,"
        " desc)") ;
    GB_BURBLE_START ("GrB_eWiseMult") ;
    GB_RETURN_IF_NULL_OR_FAULTY (semiring) ;

    //--------------------------------------------------------------------------
    // eWise multiply using the semiring multiply operator
    //--------------------------------------------------------------------------

    GB_EWISE (semiring->multiply) ;
    GB_BURBLE_END ;
    return (info) ;
}

