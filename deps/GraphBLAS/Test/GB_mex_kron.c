//------------------------------------------------------------------------------
// GB_mex_kron: C<Mask> = accum(C,kron(A,B))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "C = GB_mex_kron (C, Mask, accum, mult, A, B, desc)"

#define FREE_ALL                    \
{                                   \
    GrB_Matrix_free_(&A) ;           \
    GrB_Matrix_free_(&B) ;           \
    GrB_Matrix_free_(&C) ;           \
    GrB_Descriptor_free_(&desc) ;   \
    GrB_Matrix_free_(&Mask) ;        \
    GB_mx_put_global (true) ;       \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;
    GrB_Matrix C = NULL ;
    GrB_Matrix Mask = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_BinaryOp mult = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 6 || nargin > 7)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get C (make a deep copy)
    #define GET_DEEP_COPY \
        C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;
    #define FREE_DEEP_COPY GrB_Matrix_free_(&C) ;

    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }

    // get Mask (shallow copy)
    Mask = GB_mx_mxArray_to_Matrix (pargin [1], "Mask", false, false) ;
    if (Mask == NULL && !mxIsEmpty (pargin [1]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("Mask failed") ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [4], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get B (shallow copy)
    B = GB_mx_mxArray_to_Matrix (pargin [5], "B input", false, true) ;
    if (B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("B failed") ;
    }

    // get mult operator
    bool user_complex = (Complex != GxB_FC64)
        && (A->type == Complex || B->type == Complex) ;
    if (!GB_mx_mxArray_to_BinaryOp (&mult, pargin [3], "mult",
        C->type, user_complex) || mult == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("mult failed") ;
    }

    // get accum, if present
    user_complex = (Complex != GxB_FC64)
        && (C->type == Complex || mult->ztype == Complex) ;
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        C->type, user_complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (6), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // test all 3 variants: monoid, semiring, and binary op
    if (mult == GrB_PLUS_FP64)
    {
        // C<Mask> = accum(C,kron(A,B)), monoid variant
        METHOD (GrB_Matrix_kronecker_Monoid_ (C, Mask, accum, GrB_PLUS_MONOID_FP64,
            A, B, desc)) ;
    }
    else if (mult == GrB_TIMES_FP64)
    {
        // C<Mask> = accum(C,kron(A,B)), semiring variant
        METHOD (GrB_Matrix_kronecker_Semiring_ (C, Mask, accum, GrB_PLUS_TIMES_SEMIRING_FP64,
            A, B, desc)) ;
    }
    else if (mult == GrB_TIMES_FP32)
    {
        // C<Mask> = accum(C,kron(A,B)), binary op variant (old name)
        METHOD (GxB_kron (C, Mask, accum, mult, A, B, desc)) ;
    }
    else
    {
        // C<Mask> = accum(C,kron(A,B)), binary op variant (new name)
        METHOD (GrB_Matrix_kronecker_BinaryOp_ (C, Mask, accum, mult, A, B, desc)) ;
    }

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

