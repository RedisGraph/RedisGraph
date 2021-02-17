//------------------------------------------------------------------------------
// GB_mex_band: C = tril (triu (A,lo), hi), or with A'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Apply a select operator to a matrix

#include "GB_mex.h"

#define USAGE "C = GB_mex_band (A, lo, hi, atranspose)"

#define FREE_ALL                        \
{                                       \
    GxB_Scalar_free_(&Thunk) ;          \
    GrB_Matrix_free_(&C) ;              \
    GrB_Matrix_free_(&A) ;              \
    GxB_Scalar_free_(&Thunk_type) ;     \
    GxB_SelectOp_free_(&op) ;           \
    GrB_Descriptor_free_(&desc) ;       \
    GB_mx_put_global (true) ;           \
}

#define OK(method)                                      \
{                                                       \
    info = method ;                                     \
    if (info != GrB_SUCCESS)                            \
    {                                                   \
        FREE_ALL ;                                      \
        mexErrMsgTxt ("GraphBLAS failed") ;             \
    }                                                   \
}

typedef struct
{
    int64_t lo ;
    int64_t hi ;
} LoHi_type ; 

bool LoHi_band (GrB_Index i, GrB_Index j,
    /* x is unused: */ const void *x, const LoHi_type *thunk) ;

bool LoHi_band (GrB_Index i, GrB_Index j,
    /* x is unused: */ const void *x, const LoHi_type *thunk)
{
    int64_t i2 = (int64_t) i ;
    int64_t j2 = (int64_t) j ;
//  printf ("i %lld j %lld lo %lld hi %lld\n", i2, j2, thunk->lo, thunk->hi) ;
//  printf ("   j-i %lld\n", j2-i2) ;
    return ((thunk->lo <= (j2-i2)) && ((j2-i2) <= thunk->hi)) ;
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
    GrB_Matrix C = NULL ;
    GrB_Matrix A = NULL ;
    GxB_SelectOp op = NULL ;
    GrB_Info info ;
    GrB_Descriptor desc = NULL ;
    GxB_Scalar Thunk = NULL ;
    GrB_Type Thunk_type = NULL ;

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // check inputs
    if (nargout > 1 || nargin < 3 || nargin > 4)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // create the Thunk
    LoHi_type bandwidth  ;
    OK (GrB_Type_new (&Thunk_type, sizeof (LoHi_type))) ;

    // get lo and hi
    bandwidth.lo = (int64_t) mxGetScalar (pargin [1]) ;
    bandwidth.hi = (int64_t) mxGetScalar (pargin [2]) ;

    OK (GxB_Scalar_new (&Thunk, Thunk_type)) ;
    OK (GxB_Scalar_setElement_UDT (Thunk, (void *) &bandwidth)) ;
    OK (GxB_Scalar_wait_(&Thunk)) ;

    // get atranspose
    bool atranspose = false ;
    if (nargin > 3) atranspose = (bool) mxGetScalar (pargin [3]) ;
    if (atranspose)
    {
        OK (GrB_Descriptor_new (&desc)) ;
        OK (GxB_Desc_set (desc, GrB_INP0, GrB_TRAN)) ;
    }

    GB_MEX_TIC ;

    // create operator
    // use the user-defined operator, from the LoHi_band function
    METHOD (GxB_SelectOp_new (&op, (GxB_select_function) LoHi_band,
        NULL, Thunk_type)) ;

    GrB_Index nrows, ncols ;
    GrB_Matrix_nrows (&nrows, A) ;
    GrB_Matrix_ncols (&ncols, A) ;
    if (bandwidth.lo == 0 && bandwidth.hi == 0 && nrows == 10 && ncols == 10)
    {
        GxB_SelectOp_fprint_ (op, 3, NULL) ;
    }

    // create result matrix C
    if (atranspose)
    {
        OK (GrB_Matrix_new (&C, GrB_FP64, A->vdim, A->vlen)) ;
    }
    else
    {
        OK (GrB_Matrix_new (&C, GrB_FP64, A->vlen, A->vdim)) ;
    }

    // C<Mask> = accum(C,op(A))
    if (GB_NCOLS (C) == 1 && !atranspose)
    {
        // this is just to test the Vector version
        OK (GxB_Vector_select_((GrB_Vector) C, NULL, NULL, op, (GrB_Vector) A,
            Thunk, NULL)) ;
    }
    else
    {
        OK (GxB_Matrix_select_(C, NULL, NULL, op, A, Thunk, desc)) ;
    }

    GB_MEX_TOC ;

    // return C to MATLAB as a sparse matrix and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", false) ;

    FREE_ALL ;
}

