//------------------------------------------------------------------------------
// GrB_Matrix_apply: apply a unary operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum(C,op(A)) or accum(C,op(A'))

#include "GB_apply.h"

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

    GB_WHERE ("GrB_Matrix_apply (C, M, accum, op, A, desc)") ;
    GB_BURBLE_START ("GrB_apply") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        A_transpose, xx1, xx2) ;

    //--------------------------------------------------------------------------
    // apply the operator and optionally transpose; assemble pending tuples
    //--------------------------------------------------------------------------

    info = GB_apply (
        C,      C_replace,          // C and its descriptor
        M, Mask_comp, Mask_struct,  // mask and its descriptor
        accum,                      // optional accum for Z=accum(C,T)
        op,                         // operator to apply to the entries
        A,      A_transpose,        // A and its descriptor
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

