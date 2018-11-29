//------------------------------------------------------------------------------
// GrB_eWiseMult_Matrix: matrix element-wise operations, using set intersection
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum (C,A.*B) and variations.

#include "GB.h"

#define GB_EWISE(op)                                                        \
{                                                                           \
    /* check inputs */                                                      \
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;                                       \
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;                                       \
    GB_RETURN_IF_NULL_OR_FAULTY (B) ;                                       \
    GB_RETURN_IF_FAULTY (Mask) ;                                            \
    /* get the descriptor */                                                \
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_tran, B_tran, xx) ; \
    /* C<Mask> = accum (C,T) where T = A.*B, A'.*B, A.*B', or A'.*B' */     \
    return (GB_eWise (                                                      \
        C,          C_replace,      /* C matrix and its descriptor      */  \
        Mask,       Mask_comp,      /* Mask matrix and its descriptor   */  \
        accum,                      /* for accum (C,T)                  */  \
        op,                         /* operator that defines T=A.*B     */  \
        A,          A_tran,         /* A matrix and its descriptor      */  \
        B,          B_tran,         /* B matrix and its descriptor      */  \
        false,                      /* do eWiseMult                     */  \
        Context)) ;                                                         \
}

//------------------------------------------------------------------------------
// GrB_eWiseMult_Matrix_BinaryOp: matrix element-wise multiplication
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseMult_Matrix_BinaryOp       // C<Mask> = accum (C, A.*B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp mult,        // defines '.*' for T=A.*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseMult_Matrix_BinaryOp (C, Mask, accum, mult, A, B,"
        " desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (mult) ;

    //--------------------------------------------------------------------------
    // apply the eWise kernel (using set intersection)
    //--------------------------------------------------------------------------

    GB_EWISE (mult) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseMult_Matrix_Monoid: matrix element-wise multiplication
//------------------------------------------------------------------------------

// C<Mask> = accum (C,A.*B) and variations.

GrB_Info GrB_eWiseMult_Matrix_Monoid         // C<Mask> = accum (C, A.*B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Monoid monoid,        // defines '.*' for T=A.*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseMult_Matrix_Monoid (C, Mask, accum, monoid, A, B,"
        " desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;

    //--------------------------------------------------------------------------
    // eWise multiply using the monoid operator
    //--------------------------------------------------------------------------

    GB_EWISE (monoid->op) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseMult_Matrix_Semiring: matrix element-wise multiplication
//------------------------------------------------------------------------------

// C<Mask> = accum (C,A.*B) and variations.

GrB_Info GrB_eWiseMult_Matrix_Semiring       // C<Mask> = accum (C, A.*B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '.*' for T=A.*B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseMult_Matrix_Semiring (C, Mask, accum, semiring, A, B,"
        " desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (semiring) ;

    //--------------------------------------------------------------------------
    // eWise multiply using the semiring's multiply operator
    //--------------------------------------------------------------------------

    GB_EWISE (semiring->multiply) ;
}

