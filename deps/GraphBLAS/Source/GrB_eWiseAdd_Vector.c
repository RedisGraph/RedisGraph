//------------------------------------------------------------------------------
// GrB_eWiseAdd_Vector: vector element-wise operations, set union
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w<M> = accum (w,u+v)

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
    /* C<M> = accum (C,T) where T = A+B, A'+B, A+B', or A'+B' */            \
    info = GB_ewise (                                                       \
        (GrB_Matrix) w, C_replace,  /* w and its descriptor        */       \
        (GrB_Matrix) M, Mask_comp, Mask_struct, /* mask and its descriptor */\
        accum,                      /* accumulate operator         */       \
        op,                         /* operator that defines '+'   */       \
        (GrB_Matrix) u, false,      /* u, never transposed         */       \
        (GrB_Matrix) v, false,      /* v, never transposed         */       \
        true,                       /* eWiseAdd                    */       \
        Context)

//------------------------------------------------------------------------------
// GrB_eWiseAdd_Vector_BinaryOp: vector addition
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseAdd_Vector_BinaryOp       // w<M> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp add,         // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Vector_BinaryOp (w, M, accum, add, u, v, desc)") ;
    GB_BURBLE_START ("GrB_eWiseAdd") ;
    GB_RETURN_IF_NULL_OR_FAULTY (add) ;

    //--------------------------------------------------------------------------
    // apply the eWise kernel (using set union)
    //--------------------------------------------------------------------------

    GB_EWISE (add) ;
    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseAdd_Vector_Monoid: vector addition
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseAdd_Vector_Monoid         // w<M> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Monoid monoid,        // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Vector_Monoid (w, M, accum, monoid, u, v, desc)") ;
    GB_BURBLE_START ("GrB_eWiseAdd") ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;

    //--------------------------------------------------------------------------
    // eWise add using the monoid operator
    //--------------------------------------------------------------------------

    GB_EWISE (monoid->op) ;
    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseAdd_Vector_Semiring: vector addition
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseAdd_Vector_Semiring       // w<M> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and M
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Vector_Semiring (w, M, accum, semiring, u, v,"
        " desc)") ;
    GB_BURBLE_START ("GrB_eWiseAdd") ;
    GB_RETURN_IF_NULL_OR_FAULTY (semiring) ;

    //--------------------------------------------------------------------------
    // eWise add using the semiring monoid operator
    //--------------------------------------------------------------------------

    GB_EWISE (semiring->add->op) ;
    GB_BURBLE_END ;
    return (info) ;
}

