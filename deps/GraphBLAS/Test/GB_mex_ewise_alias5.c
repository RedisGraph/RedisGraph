//------------------------------------------------------------------------------
// GB_mex_ewise_alias5: C<M> = A+M
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "C = GB_mex_ewise_alias5 (C, M, op, A, desc)"

#define FREE_ALL                            \
{                                           \
    GB_MATRIX_FREE (&A) ;                   \
    GB_MATRIX_FREE (&M) ;                   \
    GB_MATRIX_FREE (&C) ;                   \
    GrB_free (&desc) ;                      \
    GB_mx_put_global (true, 0) ;            \
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
    GrB_Matrix C = NULL, M = NULL, A = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 4 || nargin > 5)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get C (make a deep copy)
    #define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;
    #define FREE_DEEP_COPY GB_MATRIX_FREE (&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (C->type) ;

    // get M (shallow copy)
    M = GB_mx_mxArray_to_Matrix (pargin [1], "M input", false, true) ;
    if (M == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("M failed") ;
    }

    // get op; default: NOP, default class is class(C)
    GrB_BinaryOp op ;
    if (!GB_mx_mxArray_to_BinaryOp (&op, pargin [2], "op",
        GB_NOP_opcode, cclass, C->type == Complex, C->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("op failed") ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [3], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (4), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // C<M> = A+M
    METHOD (GrB_eWiseAdd (C, M, NULL, op, A, M, desc)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

