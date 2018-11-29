//------------------------------------------------------------------------------
// GrB_mxm: matrix-matrix multiply
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C,A*B) and variations.
//
// The input matrices A and B are optionally transposed, as determined by the
// Descriptor desc.

#include "GB.h"

GrB_Info GrB_mxm                    // C<Mask> = accum (C, A*B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' and '*' for T=A*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B,
                                    // and method used for C=A*B
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_mxm (C, Mask, accum, semiring, A, B, desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (Mask) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (B) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose,
        B_transpose, AxB_method) ;

    //--------------------------------------------------------------------------
    // C<Mask> = accum (C,A*B) and variations, using the mxm kernel
    //--------------------------------------------------------------------------

    // C<Mask> = accum (C,T) where T = A*B, A'*B, A*B', or A'*B'
    return (GB_mxm (
        C,          C_replace,      // C matrix and its descriptor
        Mask,       Mask_comp,      // Mask matrix and its descriptor
        accum,                      // for accum (C,T)
        semiring,                   // semiring that defines T=A*B
        A,          A_transpose,    // A matrix and its descriptor
        B,          B_transpose,    // B matrix and its descriptor
        false,                      // use fmult(x,y), flipxy false
        AxB_method,                 // algorithm selector
        Context)) ;
}

