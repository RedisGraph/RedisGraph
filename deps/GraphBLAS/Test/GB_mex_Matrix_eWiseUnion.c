//------------------------------------------------------------------------------
// GB_mex_Matrix_eWiseUnion: C<M> = accum(C,A+B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE \
    "C = GB_mex_Matrix_eWiseUnion (C, M, accum, add, A, alpha, B, beta, desc)"

#define FREE_ALL                        \
{                                       \
    GrB_Matrix_free_(&A) ;              \
    GrB_Scalar_free_(&alpha) ;          \
    GrB_Matrix_free_(&B) ;              \
    GrB_Scalar_free_(&beta) ;           \
    GrB_Matrix_free_(&C) ;              \
    GrB_Descriptor_free_(&desc) ;       \
    GrB_Matrix_free_(&M) ;              \
    GB_mx_put_global (true) ;           \
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
    GrB_Matrix M = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_Scalar alpha = NULL, beta = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 8 || nargin > 9)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get C (make a deep copy)
    #define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;   \
    if (nargin > 7 && C != NULL) C->nvec_nonempty = -1 ;
    #define FREE_DEEP_COPY GrB_Matrix_free_(&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }

    // get M (shallow copy)
    M = GB_mx_mxArray_to_Matrix (pargin [1], "M", false, false) ;
    if (M == NULL && !mxIsEmpty (pargin [1]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("M failed") ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [4], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get alpha
    alpha = GB_mx_get_Scalar (pargin [5]) ; 
    if (alpha == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("alpha failed") ;
    }

    // get B (shallow copy)
    B = GB_mx_mxArray_to_Matrix (pargin [6], "B input", false, true) ;
    if (B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("B failed") ;
    }

    // get beta
    beta = GB_mx_get_Scalar (pargin [7]) ; 
    if (beta == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("beta failed") ;
    }

    // get add operator
    bool user_complex = (Complex != GxB_FC64)
        && (A->type == Complex || B->type == Complex) ;
    GrB_BinaryOp add ;
    if (!GB_mx_mxArray_to_BinaryOp (&add, pargin [3], "add",
        C->type, user_complex) || add == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("add failed") ;
    }

    // get accum, if present
    user_complex = (Complex != GxB_FC64)
        && (C->type == Complex || add->ztype == Complex) ;
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        C->type, user_complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (8), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // C<M> = accum(C,A+B)
    METHOD (GxB_Matrix_eWiseUnion(C, M, accum, add, A, alpha, B, beta,
        desc)) ;

    // return C as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

