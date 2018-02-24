//------------------------------------------------------------------------------
// GrB_vxm: vector-matrix multiply
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w'<mask'> = accum (w',t) where t = u'*A or u'*A'
// Rows w', u', and mask' are simply columns w, u, and mask.  Thus:
// w<mask> = accum (w,t) where t = A'*u or A*u, but with the multiply operator
// flipped.  The input descriptor for A, inp1, is also negated.

#include "GB.h"

GrB_Info GrB_vxm                    // w'<Mask> = accum (w, u'*A)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for w=accum(w,t)
    const GrB_Semiring semiring,    // defines '+' and '*'
    const GrB_Vector u,             // first input:  vector u
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for w, mask, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_vxm (w, mask, accum, semiring, u, A, desc)") ;

    RETURN_IF_NULL_OR_UNINITIALIZED (w) ;
    RETURN_IF_UNINITIALIZED (mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (u) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, ignore, A_transpose) ;

    //--------------------------------------------------------------------------
    // w'<mask'> = accum (w',u'*A) and variations, using the mxm kernel
    //--------------------------------------------------------------------------

    // w, mask, and u are passed as matrices to GB_mxm
    // A and u are swapped, and A_transpose is negated:
    //      u'*A  == A'*u
    //      u'*A' == A*u
    // Since A and U are swapped, in all the matrix multiply kernels
    // fmult(y,x) must be used instead of fmult(x,y).

    return (GB_mxm (
        (GrB_Matrix) w,     C_replace,      // w and its descriptor
        (GrB_Matrix) mask,  Mask_comp,      // mask and its descriptor
        accum,                              // for accum (w,t)
        semiring,                           // semiring that defines t=u*A
        A,                  !A_transpose,   // allow A to be transposed
        (GrB_Matrix) u,     false,          // u is never transposed
        true)) ;                            // flipxy: fmult(y,x)
}

