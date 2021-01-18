//------------------------------------------------------------------------------
// GB_mex_rdiv2: compute C=A*B with the rdiv2 operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This is for testing only.  See GrB_mxm instead.  Returns a plain MATLAB
// matrix, in double.  The semiring is plus-rdiv2 where plus is the 
// built-in GrB_PLUS_FP64 operator, and rdiv2 is z=y/x with y float and x
// double.  The input matrix B is typecasted here to GrB_FP32.

#include "GB_mex.h"

#define USAGE "C = GB_mex_rdiv2 (A, B, atrans, btrans, axb_method, flipxy, C_scalar)"

#define FREE_ALL                            \
{                                           \
    GrB_Matrix_free_(&A) ;                   \
    GrB_Matrix_free_(&B) ;                   \
    GrB_Matrix_free_(&B64) ;                 \
    GrB_Matrix_free_(&C) ;                   \
    GrB_Matrix_free_(&T) ;                   \
    GrB_BinaryOp_free_(&My_rdiv2) ;         \
    GrB_Semiring_free_(&My_plus_rdiv2) ;    \
    GB_mx_put_global (true) ;               \
}

//------------------------------------------------------------------------------

GrB_Info info ;
bool malloc_debug = false ;
bool ignore = false, ignore2 = false ;
bool atranspose = false ;
bool btranspose = false ;
GrB_Matrix A = NULL, B = NULL, B64 = NULL, C = NULL, T = NULL ;
int64_t anrows = 0 ;
int64_t ancols = 0 ;
int64_t bnrows = 0 ;
int64_t bncols = 0 ;
GrB_Desc_Value AxB_method = GxB_DEFAULT ;
bool flipxy = false ;
bool done_in_place = false ;
double C_scalar = 0 ;

GrB_Info axb (GB_Context Context) ;

GrB_Semiring My_plus_rdiv2 = NULL ;
GrB_BinaryOp My_rdiv2 = NULL ;

void my_rdiv2 (double *z, const double *x, const float *y) ;

void my_rdiv2 (double *z, const double *x, const float *y)
{
    (*z) = ((double) (*y)) / (*x) ;
}

//------------------------------------------------------------------------------

GrB_Info axb (GB_Context Context)
{
    // create the rdiv2 operator
    info = GrB_BinaryOp_new (&My_rdiv2, my_rdiv2, GrB_FP64, GrB_FP64, GrB_FP32);
    GrB_BinaryOp_wait_(&My_rdiv2) ;
    if (info != GrB_SUCCESS) return (info) ;
    info = GrB_Semiring_new (&My_plus_rdiv2, GxB_PLUS_FP64_MONOID, My_rdiv2) ;
    if (info != GrB_SUCCESS)
    {
        GrB_BinaryOp_free_(&My_rdiv2) ;
        return (info) ;
    }

    bool do_in_place = (C_scalar != 0) ;
    C = NULL ;

    if (do_in_place)
    {
        // construct the result matrix and fill it with the scalar
        GrB_Index cnrows = anrows ;
        GrB_Index cncols = bncols ;
        info = GrB_Matrix_new (&C, GrB_FP64, cnrows, cncols) ;
        if (info != GrB_SUCCESS)
        {
            GrB_BinaryOp_free_(&My_rdiv2) ;
            GrB_Semiring_free_(&My_plus_rdiv2) ;
            return (info) ;
        }
        info = GrB_Matrix_assign_FP64_(C, NULL, NULL, C_scalar,
            GrB_ALL, cnrows, GrB_ALL, cncols, NULL) ;
        if (info != GrB_SUCCESS) 
        {
            GrB_BinaryOp_free_(&My_rdiv2) ;
            GrB_Semiring_free_(&My_plus_rdiv2) ;
            GrB_Matrix_free_(&C) ;
            return (info) ;
        }
    }

    // C = A*B or C += A*B
    info = GB_AxB_meta (&T, C,  // can be done in place if C != NULL
        false,      // C_replace
        true,       // CSC
        NULL,       // no MT returned
        NULL,       // no Mask
        false,      // mask not complemented
        false,      // mask not structural
        (do_in_place) ? GrB_PLUS_FP64 : NULL,   // accum
        A, B,
        My_plus_rdiv2,
        atranspose,
        btranspose,
        flipxy,
        &ignore,    // mask_applied
        &done_in_place,
        AxB_method,
        true,       // do the sort
        Context) ;

    if (info == GrB_SUCCESS)
    {
        if (done_in_place != do_in_place)
        {
            mexErrMsgTxt ("failure: not in place as expected\n") ;
        }
        if (!done_in_place)
        {
            GrB_Matrix_free_(&C) ;
            C = T ;
            T = NULL ;
        }
    }
    else
    {
        GrB_Matrix_free_(&C) ;
        GrB_Matrix_free_(&T) ;
    }

    GrB_BinaryOp_free_(&My_rdiv2) ;
    GrB_Semiring_free_(&My_plus_rdiv2) ;

    return (info) ;
}

//------------------------------------------------------------------------------

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    info = GrB_SUCCESS ;
    malloc_debug = GB_mx_get_global (true) ;
    ignore = false ;
    A = NULL ;
    B = NULL ;
    B64 = NULL ;
    C = NULL ;

    My_rdiv2 = NULL ;
    My_plus_rdiv2 = NULL ;

    GB_CONTEXT (USAGE) ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 7)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A and B64
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, true) ;
    B64 = GB_mx_mxArray_to_Matrix (pargin [1], "B", false, true) ;
    if (A == NULL || B64 == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    if (!A->is_csc || !B64->is_csc)
    {
        mexErrMsgTxt ("A and B must be in CSC format") ;
    }

    // get the atranspose option
    GET_SCALAR (2, bool, atranspose, false) ;

    // get the btranspose option
    GET_SCALAR (3, bool, btranspose, false) ;

    // get the axb_method
    // 0 or not present: default
    // 1001: Gustavson
    // 1003: dot
    // 1004: hash
    // 1005: saxpy
    GET_SCALAR (4, GrB_Desc_Value, AxB_method, GxB_DEFAULT) ;

    if (! ((AxB_method == GxB_DEFAULT) ||
        (AxB_method == GxB_AxB_GUSTAVSON) ||
        (AxB_method == GxB_AxB_HASH) ||
        (AxB_method == GxB_AxB_SAXPY) ||
        (AxB_method == GxB_AxB_DOT)))
    {
        mexErrMsgTxt ("unknown method") ;
    }

    // get the flipxy option
    GET_SCALAR (5, bool, flipxy, false) ;

    // get the C_scalar
    GET_SCALAR (6, double, C_scalar, 0) ;

    // determine the dimensions
    anrows = (atranspose) ? GB_NCOLS (A) : GB_NROWS (A) ;
    ancols = (atranspose) ? GB_NROWS (A) : GB_NCOLS (A) ;
    bnrows = (btranspose) ? GB_NCOLS (B64) : GB_NROWS (B64) ;
    bncols = (btranspose) ? GB_NROWS (B64) : GB_NCOLS (B64) ;
    if (ancols != bnrows)
    {
        FREE_ALL ;
        mexErrMsgTxt ("invalid dimensions") ;
    }

    if (atranspose && btranspose && C_scalar != 0)
    {
        C_scalar = 0 ;
    }

    // convert B64 (double) to B (float)
    GrB_Matrix_new (&B, GrB_FP32, bnrows, bncols) ;
    GrB_Matrix_assign_(B, NULL, NULL, B64, GrB_ALL, 0, GrB_ALL, 0, NULL) ;

    // B must be completed
    GrB_Matrix_wait (&B) ;

    METHOD (axb (Context)) ;

    // return C to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C AxB result", false) ;

    FREE_ALL ;
}

