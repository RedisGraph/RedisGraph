//------------------------------------------------------------------------------
// GxB_kron: Kronecker product
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_kron.h"

GrB_Info GxB_kron                   // C<M> = accum (C, kron(A,B))
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '*' for T=kron(A,B)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, M, A, and B
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_kron (C, M, accum, op, A, B, desc)") ;
    GB_BURBLE_START ("GxB_kron") ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        A_tran, B_tran, xx) ;

    //--------------------------------------------------------------------------
    // C = kron(A,B)
    //--------------------------------------------------------------------------

    // C<M> = accum (C,T) where T = kron(A,B), or with A' and/or B'
    info = GB_kron (
        C,          C_replace,      // C matrix and its descriptor
        M, Mask_comp, Mask_struct,  // mask matrix and its descriptor
        accum,                      // for accum (C,T)
        op,                         // operator that defines T=kron(A,B)
        A,          A_tran,         // A matrix and its descriptor
        B,          B_tran,         // B matrix and its descriptor
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

