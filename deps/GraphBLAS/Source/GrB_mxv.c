//------------------------------------------------------------------------------
// GrB_mxv: matrix-vector multiply
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w<mask> = accum (w,t) where t = A*u or A'*u (u is never transposed)
//
// The input matrix A is optionally transposed, as determined by the
// Descriptor desc.

#include "GB.h"

GrB_Info GrB_mxv                    // w<Mask> = accum (w, A*u)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '+' and '*' for A*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Vector u,             // second input: vector u
    const GrB_Descriptor desc       // descriptor for w, mask, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_mxv (w, mask, accum, semiring, A, u, desc)") ;

    RETURN_IF_NULL_OR_UNINITIALIZED (w) ;
    RETURN_IF_UNINITIALIZED (mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (u) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose, ignore) ;

    //--------------------------------------------------------------------------
    // w<mask> = accum (w,A*u) and variations, using the mxm kernel
    //--------------------------------------------------------------------------

    // w, mask, and u are passed as matrices to GB_mxm.
    // The In1_transpose descriptor is ignored.

    return (GB_mxm (
        (GrB_Matrix) w,     C_replace,      // w and its descriptor
        (GrB_Matrix) mask,  Mask_comp,      // mask and its descriptor
        accum,                              // for accum (w,t)
        semiring,                           // semiring that defines t=A*u
        A,                  A_transpose,    // allow A to be transposed
        (GrB_Matrix) u,     false,          // u is never transposed
        false)) ;                           // fmult(x,y), flipxy false
}

