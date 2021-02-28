//------------------------------------------------------------------------------
// GrB_eWiseAdd_Matrix: matrix element-wise operations, set union
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C,A+B) and variations.

#include "GB_ewise.h"

#define GB_EWISE(op)                                                        \
    /* check inputs */                                                      \
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;                                       \
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;                                       \
    GB_RETURN_IF_NULL_OR_FAULTY (B) ;                                       \
    GB_RETURN_IF_FAULTY (M) ;                                               \
    /* get the descriptor */                                                \
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,       \
        A_tran, B_tran, xx) ;                                               \
    /* C<M> = accum (C,T) where T = A+B, A'+B, A+B', or A'+B' */            \
    info = GB_ewise (                                                       \
        C,              C_replace,  /* C and its descriptor        */       \
        M, Mask_comp, Mask_struct,  /* mask and its descriptor     */       \
        accum,                      /* accumulate operator         */       \
        op,                         /* operator that defines '+'   */       \
        A,              A_tran,     /* A matrix and its descriptor */       \
        B,              B_tran,     /* B matrix and its descriptor */       \
        true,                       /* eWiseAdd                    */       \
        Context) ;

//------------------------------------------------------------------------------
// GrB_eWiseAdd_Matrix_BinaryOp: matrix addition
//------------------------------------------------------------------------------

GrB_Info GrB_eWiseAdd_Matrix_BinaryOp       // C<M> = accum (C, A+B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp add,         // defines '+' for T=A+B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, M, A, and B
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Matrix_BinaryOp (C, M, accum, add, A, B, desc)") ;
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
// GrB_eWiseAdd_Matrix_Monoid: matrix addition
//------------------------------------------------------------------------------

// C<M> = accum (C,A+B) and variations.

GrB_Info GrB_eWiseAdd_Matrix_Monoid         // C<M> = accum (C, A+B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Monoid monoid,        // defines '+' for T=A+B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, M, A, and B
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Matrix_Monoid (C, M, accum, monoid, A, B, desc)") ;
    GB_BURBLE_START ("GrB_eWiseAdd") ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;

    //--------------------------------------------------------------------------
    // eWiseAdd using the monoid operator
    //--------------------------------------------------------------------------

    GB_EWISE (monoid->op) ;
    GB_BURBLE_END ;
    return (info) ;
}

//------------------------------------------------------------------------------
// GrB_eWiseAdd_Matrix_Semiring: matrix addition
//------------------------------------------------------------------------------

// C<M> = accum (C,A+B) and variations.

GrB_Info GrB_eWiseAdd_Matrix_Semiring       // C<M> = accum (C, A+B)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' for T=A+B
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Matrix B,             // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, M, A, and B
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_eWiseAdd_Matrix_Semiring (C, M, accum, semiring, A, B,"
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

