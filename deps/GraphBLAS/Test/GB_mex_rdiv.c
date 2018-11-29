//------------------------------------------------------------------------------
// GB_mex_rdiv: compute C=A*B with the rdiv operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is for testing only.  See GrB_mxm instead.  Returns a plain MATLAB
// matrix, in double.  The semiring is plus-rdiv-fp64 where plus is the 
// built-in GrB_PLUS_FP64 operator, and rdiv is z=y/x in double.

#include "GB_mex.h"

#define USAGE "C = GB_mex_rdiv (A, B, axb_method, cprint)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&B) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GB_Sauna_free (&Sauna) ;            \
    GrB_free (&My_rdiv) ;               \
    GrB_free (&My_plus_rdiv) ;          \
    GB_mx_put_global (true, 0) ;        \
}

//------------------------------------------------------------------------------

GrB_Info info ;
bool malloc_debug = false ;
bool ignore = false ;
bool cprint = false ;
GrB_Matrix A = NULL, B = NULL, C = NULL ;
int64_t anrows = 0 ;
int64_t ancols = 0 ;
int64_t bnrows = 0 ;
int64_t bncols = 0 ;
GrB_Desc_Value AxB_method = GxB_DEFAULT, AxB_method_used ;
GB_Sauna Sauna = NULL ;

#ifndef MY_RDIV
GrB_Semiring My_plus_rdiv = NULL ;
GrB_BinaryOp My_rdiv = NULL ;

void my_rdiv
(
    double *z,
    const double *x,
    const double *y
)
{
    (*z) = (*y) / (*x) ;
}
#endif

//------------------------------------------------------------------------------

GrB_Info axb (GB_Context Context, bool cprint)
{
    #ifndef MY_RDIV
    // create the rdiv operator
    info = GrB_BinaryOp_new (&My_rdiv, my_rdiv, GrB_FP64, GrB_FP64, GrB_FP64) ;
    if (info != GrB_SUCCESS) return (info) ;
    info = GrB_Semiring_new (&My_plus_rdiv, GxB_PLUS_FP64_MONOID, My_rdiv) ;
    if (info != GrB_SUCCESS)
    {
        GrB_free (&My_rdiv) ;
        return (info) ;
    }
    #else
    // printf ("using precompiled semiring %p\n", My_plus_rdiv) ;
    #endif

    // GB_check (My_plus_rdiv, "My_plus_rdiv", 3) ;

    // C = A*B
    info = GB_AxB_meta (&C, true /* CSC */,
        NULL /* no MT returned */,
        NULL /* no Mask */,
        A, B, My_plus_rdiv, false, false, false, &ignore,
        AxB_method, &AxB_method_used, &Sauna, Context) ;

    if (C != NULL)
    {
        C->AxB_method_used = AxB_method_used ;
        if (cprint) GxB_print (C, GxB_COMPLETE) ;
    }

    // does nothing if the objects are pre-compiled
    GrB_free (&My_rdiv) ;
    GrB_free (&My_plus_rdiv) ;

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

    #ifndef MY_RDIV
    My_rdiv = NULL ;
    My_plus_rdiv = NULL ;
    #endif

    GB_WHERE (USAGE) ;

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
    // 1002: heap
    // 1003: dot
    GET_SCALAR (2, GrB_Desc_Value, AxB_method, GxB_DEFAULT) ;

    // get the cprint flag
    GET_SCALAR (3, bool, cprint, false) ;

    if (! ((AxB_method == GxB_DEFAULT) ||
        (AxB_method == GxB_AxB_GUSTAVSON) ||
        (AxB_method == GxB_AxB_HEAP) ||
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
    GrB_finalize ( ) ;
}

