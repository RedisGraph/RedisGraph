//------------------------------------------------------------------------------
// GrB_eWiseAdd_Vector: vector element-wise operations, set union
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w<mask> = accum (w,u+v)

#include "GB.h"

#define GB_EWISE(op)                                                        \
{                                                                           \
    /* check inputs */                                                      \
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;                                       \
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;                                       \
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;                                       \
    GB_RETURN_IF_FAULTY (mask) ;                                            \
    ASSERT (GB_VECTOR_OK (w)) ;                                             \
    ASSERT (GB_VECTOR_OK (u)) ;                                             \
    ASSERT (GB_VECTOR_OK (v)) ;                                             \
    ASSERT (mask == NULL || GB_VECTOR_OK (mask)) ;                          \
    /* get the descriptor */                                                \
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, xx1, xx2, xx3) ;   \
    /* C<mask> = accum (C,T) where T = A+B, A'+B, A+B', or A'+B' */         \
    return (GB_eWise (                                                      \
        (GrB_Matrix) w,    C_replace,   /* w and its descriptor        */   \
        (GrB_Matrix) mask, Mask_comp,   /* mask and its descriptor     */   \
        accum,                          /* for accum (w,t)             */   \
        op,                             /* operator that defines t=u+v */   \
        (GrB_Matrix) u,    false,       /* u, never transposed         */   \
        (GrB_Matrix) v,    false,       /* v, never transposed         */   \
        true,                           /* eWiseAdd                    */   \
        Context)) ;                                                         \
}

//------------------------------------------------------------------------------
// GrB_eWiseAdd_Vector_BinaryOp: vector addition
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseAdd_Vector_BinaryOp       // w<mask> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_BinaryOp add,         // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Vector_BinaryOp (w, mask, accum, add, u, v, desc)");
    GB_RETURN_IF_NULL_OR_FAULTY (add) ;

    //--------------------------------------------------------------------------
    // apply the eWise kernel (using set union)
    //--------------------------------------------------------------------------

    GB_EWISE (add) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseAdd_Vector_Monoid: vector addition
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseAdd_Vector_Monoid         // w<mask> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Monoid monoid,        // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Vector_Monoid (w, mask, accum, monoid, u, v,"
        " desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;

    //--------------------------------------------------------------------------
    // eWise add using the monoid operator
    //--------------------------------------------------------------------------

    GB_EWISE (monoid->op) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseAdd_Vector_Semiring: vector addition
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseAdd_Vector_Semiring       // w<Mask> = accum (w, u+v)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '+' for t=u+v
    const GrB_Vector u,             // first input:  vector u
    const GrB_Vector v,             // second input: vector v
    const GrB_Descriptor desc       // descriptor for w and mask
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Vector_Semiring (w, mask, accum, semiring, u, v,"
        " desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (semiring) ;

    //--------------------------------------------------------------------------
    // eWise add using the semiring monoid operator
    //--------------------------------------------------------------------------

    GB_EWISE (semiring->add->op) ;
}
