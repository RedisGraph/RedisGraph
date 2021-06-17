//------------------------------------------------------------------------------
// GB_mex_rdiv: compute C=A*B with the rdiv operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This is for testing only.  See GrB_mxm instead.  Returns a plain MATLAB
// matrix, in double.  The semiring is plus-rdiv-fp64 where plus is the 
// built-in GrB_PLUS_FP64 operator, and rdiv is z=y/x in double.

#include "GB_mex.h"

#define USAGE "C = GB_mex_rdiv (A, B, axb_method, cprint)"

#define FREE_ALL                        \
{                                       \
    GrB_Matrix_free_(&A) ;               \
    GrB_Matrix_free_(&B) ;               \
    GrB_Matrix_free_(&C) ;               \
    GrB_BinaryOp_free_(&My_rdiv) ;      \
    GrB_Semiring_free_(&My_plus_rdiv) ; \
    GB_mx_put_global (true) ;           \
}

//------------------------------------------------------------------------------

GrB_Info info ;
bool malloc_debug = false ;
bool ignore = false, ignore2 = false ;
bool cprint = false ;
GrB_Matrix A = NULL, B = NULL, C = NULL ;
int64_t anrows = 0 ;
int64_t ancols = 0 ;
int64_t bnrows = 0 ;
int64_t bncols = 0 ;
GrB_Desc_Value AxB_method = GxB_DEFAULT ;

GrB_Info axb (GB_Context Context, bool cprint) ;

GrB_Semiring My_plus_rdiv = NULL ;
GrB_BinaryOp My_rdiv = NULL ;

void my_rdiv (double *z, const double *x, const double *y) ;

void my_rdiv (double *z, const double *x, const double *y)
{
    (*z) = (*y) / (*x) ;
}

//------------------------------------------------------------------------------

GrB_Info axb (GB_Context Context, bool cprint)
{
    // create the rdiv operator
    info = GrB_BinaryOp_new (&My_rdiv, my_rdiv, GrB_FP64, GrB_FP64, GrB_FP64) ;
    if (info != GrB_SUCCESS) return (info) ;
    GrB_BinaryOp_wait_(&My_rdiv) ;
    if (info != GrB_SUCCESS) return (info) ;
    info = GrB_Semiring_new (&My_plus_rdiv, GxB_PLUS_FP64_MONOID, My_rdiv) ;
    if (info != GrB_SUCCESS)
    {
        GrB_BinaryOp_free_(&My_rdiv) ;
        return (info) ;
    }

    // C = A*B
    info = GB_AxB_meta (&C, NULL,       // C cannot be computed in place
        false,      // C_replace
        true,       // CSC
        NULL,       // no MT returned
        NULL,       // no Mask
        false,      // mask not complemented
        false,      // mask not structural
        NULL,       // no accum
        A, B,
        My_plus_rdiv,
        false,      // no A transpose
        false,      // no B transpose
        false,      // no flipxy
        &ignore,    // mask_applied
        &ignore2,   // done_in_place
        AxB_method,
        true,       // do the sort
        Context) ;

    if (C != NULL)
    {
        if (cprint) GxB_Matrix_fprint_(C, GxB_COMPLETE, NULL) ;
    }

    GrB_BinaryOp_free_(&My_rdiv) ;
    GrB_Semiring_free_(&My_plus_rdiv) ;

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
    C = NULL ;

    My_rdiv = NULL ;
    My_plus_rdiv = NULL ;

    GB_CONTEXT (USAGE) ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 4)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A and B
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, true) ;
    B = GB_mx_mxArray_to_Matrix (pargin [1], "B", false, true) ;
    if (A == NULL || B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    if (!A->is_csc || !B->is_csc)
    {
        mexErrMsgTxt ("A and B must be in CSC format") ;
    }

    // get the axb_method
    // 0 or not present: default
    // 1001: Gustavson
    // 1003: dot
    // 1004: hash
    // 1005: saxpy
    GET_SCALAR (2, GrB_Desc_Value, AxB_method, GxB_DEFAULT) ;

    // get the cprint flag
    GET_SCALAR (3, bool, cprint, false) ;

    if (! ((AxB_method == GxB_DEFAULT) ||
        (AxB_method == GxB_AxB_GUSTAVSON) ||
        (AxB_method == GxB_AxB_HASH) ||
        (AxB_method == GxB_AxB_SAXPY) ||
        (AxB_method == GxB_AxB_DOT)))
    {
        mexErrMsgTxt ("unknown method") ;
    }

    // determine the dimensions
    anrows = GB_NROWS (A) ;
    ancols = GB_NCOLS (A) ;
    bnrows = GB_NROWS (B) ;
    bncols = GB_NCOLS (B) ;
    if (ancols != bnrows)
    {
        FREE_ALL ;
        mexErrMsgTxt ("invalid dimensions") ;
    }

    METHOD (axb (Context, cprint)) ;

    // return C to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C AxB result", false) ;

    FREE_ALL ;
}

