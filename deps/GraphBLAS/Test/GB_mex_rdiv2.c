//------------------------------------------------------------------------------
// GB_mex_rdiv2: compute C=A*B with the rdiv2 operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is for testing only.  See GrB_mxm instead.  Returns a plain MATLAB
// matrix, in double.  The semiring is plus-rdiv2 where plus is the 
// built-in GrB_PLUS_FP64 operator, and rdiv2 is z=y/x with y float and x
// double.  The input matrix B is typecasted here to GrB_FP32.

#include "GB_mex.h"

#define USAGE "C = GB_mex_AxB (A, B, atrans, btrans, axb_method, flipxy)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&B) ;               \
    GB_MATRIX_FREE (&B64) ;             \
    GB_MATRIX_FREE (&C) ;               \
    GB_Sauna_free (&Sauna) ;            \
    GrB_free (&My_rdiv2) ;              \
    GrB_free (&My_plus_rdiv2) ;         \
    GB_mx_put_global (true, 0) ;        \
}

//------------------------------------------------------------------------------

GrB_Info info ;
bool malloc_debug = false ;
bool ignore = false ;
bool atranspose = false ;
bool btranspose = false ;
GrB_Matrix A = NULL, B = NULL, B64 = NULL, C = NULL ;
int64_t anrows = 0 ;
int64_t ancols = 0 ;
int64_t bnrows = 0 ;
int64_t bncols = 0 ;
GrB_Desc_Value AxB_method = GxB_DEFAULT, AxB_method_used ;
bool flipxy = false ;
GB_Sauna Sauna = NULL ;

#ifndef MY_RDIV
GrB_Semiring My_plus_rdiv2 = NULL ;
GrB_BinaryOp My_rdiv2 = NULL ;

void my_rdiv2
(
    double *z,
    const double *x,
    const float *y
)
{
    (*z) = (*y) / (*x) ;
}
#endif

//------------------------------------------------------------------------------

GrB_Info axb (GB_Context Context)
{
    #ifndef MY_RDIV
    // create the rdiv2 operator
    info = GrB_BinaryOp_new (&My_rdiv2, my_rdiv2, GrB_FP64, GrB_FP64, GrB_FP32);
    if (info != GrB_SUCCESS) return (info) ;
    info = GrB_Semiring_new (&My_plus_rdiv2, GxB_PLUS_FP64_MONOID, My_rdiv2) ;
    if (info != GrB_SUCCESS)
    {
        GrB_free (&My_rdiv2) ;
        return (info) ;
    }
    #else
    // printf ("using precompiled semiring %p\n", My_plus_rdiv2) ;
    #endif

    // GB_check (My_plus_rdiv2, "My_plus_rdiv2", GB0) ;

    // C = A*B (not user-callable)
    info = GB_AxB_meta (&C, true /* CSC */,
        NULL /* no MT returned */,
        NULL /* no Mask */,
        A, B, My_plus_rdiv2,
        atranspose, btranspose, flipxy, &ignore,
        AxB_method, &AxB_method_used, &Sauna, Context) ;

    // does nothing if the objects are pre-compiled
    GrB_free (&My_rdiv2) ;
    GrB_free (&My_plus_rdiv2) ;

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

    #ifndef MY_RDIV
    My_rdiv2 = NULL ;
    My_plus_rdiv2 = NULL ;
    #endif

    GB_WHERE (USAGE) ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 6)
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
    // 1002: heap
    // 1003: dot
    GET_SCALAR (4, GrB_Desc_Value, AxB_method, GxB_DEFAULT) ;

    if (! ((AxB_method == GxB_DEFAULT) ||
        (AxB_method == GxB_AxB_GUSTAVSON) ||
        (AxB_method == GxB_AxB_HEAP) ||
        (AxB_method == GxB_AxB_DOT)))
    {
        mexErrMsgTxt ("unknown method") ;
    }

    // get the flipxy option
    GET_SCALAR (5, bool, flipxy, false) ;

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

    // convert B64 (double) to B (float)
    GrB_Matrix_new (&B, GrB_FP32, bnrows, bncols) ;
    GrB_assign (B, NULL, NULL, B64, GrB_ALL, 0, GrB_ALL, 0, NULL) ;

    // B must be completed for GB_AxB_meta to work
    GrB_Index nvals ;
    GrB_Matrix_nvals (&nvals, B) ;
    // GB_check (B, "B float", GB0) ;
    // GB_check (B64, "B64 double", GB0) ;

    METHOD (axb (Context)) ;

    // return C to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C AxB result", false) ;

    FREE_ALL ;
    GrB_finalize ( ) ;
}

