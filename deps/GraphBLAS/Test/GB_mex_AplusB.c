//------------------------------------------------------------------------------
// GB_mex_AplusB: compute C=A+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is for testing only.  See GrB_eWiseAdd instead.  Returns a plain MATLAB
// matrix, in double.

#include "GB_mex.h"

#define USAGE "C = GB_mex_AplusB (A, B, op)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&B) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GB_mx_put_global (true, 0) ;        \
}


void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    // double tic2 [2] ;
    // simple_tic (tic2) ;

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;
    GrB_Matrix C = NULL ;
    GrB_BinaryOp op = NULL ;

    GB_WHERE (USAGE) ;

    // check inputs
    if (nargout > 1 || nargin != 3)
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
    mxClassID aclass = GB_mx_Type_to_classID (A->type) ;

    // get op; default: NOP, default class is class(A)
    if (!GB_mx_mxArray_to_BinaryOp (&op, pargin [2], "op",
        GB_NOP_opcode, aclass, A->type == Complex, B->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("op failed") ;
    }

    // printf ("time so far: %g\n", simple_toc (tic2)) ;
    // simple_tic (tic2) ;

    // C = A+B using the op.  No mask
    METHOD (GB_add (&C, A->type, true, NULL, false, A, B, op, Context)) ;

    // return C to MATLAB as a plain sparse matrix
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C AplusB result", false) ;

    FREE_ALL ;
    // printf ("time wrapup: %g\n", simple_toc (tic2)) ;
}

