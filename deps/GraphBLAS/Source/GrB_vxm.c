//------------------------------------------------------------------------------
// GrB_vxm: vector-matrix multiply
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// w'<M'> = accum (w',t) where t = u'*A or u'*A'

// Rows w', u', and M' are simply columns w, u, and M.  Thus:
// w<M> = accum (w,t) where t = A'*u or A*u, but with the multiply operator
// flipped.  The input descriptor for A, inp1, is also negated.

#include "GB_mxm.h"

GrB_Info GrB_vxm                    // w'<M> = accum (w, u'*A)
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector M,             // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Semiring semiring,    // defines '+' and '*' for matrix multiply
    const GrB_Vector u,             // first input:  vector u
    const GrB_Matrix A,             // second input: matrix A
    const GrB_Descriptor desc       // descriptor for w, M, and A
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_vxm (w, M, accum, semiring, u, A, desc)") ;
    GB_BURBLE_START ("GrB_vxm") ;
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (M == NULL || GB_VECTOR_OK (M)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        xx, A_transpose, AxB_method) ;

    //--------------------------------------------------------------------------
    // w'<M'> = accum (w',u'*A) and variations, using the mxm kernel
    //--------------------------------------------------------------------------

    // w, M, and u are passed as matrices to GB_mxm
    // A and u are swapped, and A_transpose is negated:
    //      u'*A  == A'*u
    //      u'*A' == A*u
    // Since A and u are swapped, in all the matrix multiply kernels
    // fmult(y,x) must be used instead of fmult(x,y).

    info = GB_mxm (
        (GrB_Matrix) w,     C_replace,      // w and its descriptor
        (GrB_Matrix) M, Mask_comp, Mask_struct, // mask and its descriptor
        accum,                              // for accum (w,t)
        semiring,                           // definition of matrix multiply
        A,                  !A_transpose,   // allow A to be transposed
        (GrB_Matrix) u,     false,          // u is never transposed
        true,                               // flipxy: fmult(y,x)
        AxB_method,                         // algorithm selector
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

