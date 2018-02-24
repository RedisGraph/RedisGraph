//------------------------------------------------------------------------------
// GB_mex_AplusB: compute C=A+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is for testing only.  See GrB_eWiseAdd instead.  Returns a plain MATLAB
// matrix, in double.

#include "GB_mex.h"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&B) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GB_mx_put_global (malloc_debug) ;   \
}


void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global ( ) ;
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;
    GrB_Matrix C = NULL ;
    GrB_BinaryOp op = NULL ;

    // check inputs
    if (nargout > 1 || nargin != 3)
    {
        mexErrMsgTxt ("Usage: C = GB_mex_AplusB (A, B, op)") ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A and B
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false) ;
    B = GB_mx_mxArray_to_Matrix (pargin [1], "B", false) ;
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

    // create the GraphBLAS output matrix C; same type as A
    METHOD (GrB_Matrix_new (&C, A->type, A->nrows, A->ncols)) ;

    // C = A+B using the op
    METHOD (GB_Matrix_add (C, A, B, op)) ;

    // return C to MATLAB as a plain sparse matrix
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C AplusB result", false) ;

    FREE_ALL ;
}

