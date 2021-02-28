//------------------------------------------------------------------------------
// GrB_Vector_apply: apply a unary operator to a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_apply.h"

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

    GB_WHERE ("GrB_Vector_apply (w, M, accum, op, u, desc)") ;
    GB_BURBLE_START ("GrB_apply") ;
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;

    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (M == NULL || GB_VECTOR_OK (M)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        xx1, xx2, xx3) ;

    //--------------------------------------------------------------------------
    // apply the operator; do not transpose
    //--------------------------------------------------------------------------

    info = GB_apply (
        (GrB_Matrix) w,     C_replace,      // w and its descriptor
        (GrB_Matrix) M, Mask_comp, Mask_struct, // mask and its descriptor
        accum,                              // optional accum for Z=accum(C,T)
        op,                                 // operator to apply to the entries
        (GrB_Matrix) u,     false,          // u, not transposed
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

