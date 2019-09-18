//------------------------------------------------------------------------------
// GB_mex_eWiseMult_first: C<Mask> = accum(C,first(A,B))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is a simplified version of GB_mex_eWiseMult_Matrix.  The op is
// always GrB_FIRST_FP64.

#include "GB_mex.h"

#define USAGE "C = GB_mex_eWiseMult_first (C, Mask, accum, [ ], A, B, desc)"

#define FREE_ALL                    \
{                                   \
    GB_MATRIX_FREE (&A) ;           \
    GB_MATRIX_FREE (&B) ;           \
    GB_MATRIX_FREE (&C) ;           \
    GrB_free (&desc) ;              \
    GB_MATRIX_FREE (&Mask) ;        \
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

    bool malloc_debug = GB_mx_get_global (true) ;
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;
    GrB_Matrix C = NULL ;
    GrB_Matrix Mask = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 6 || nargin > 7)
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
    GrB_BinaryOp mult ;
    mult = GrB_FIRST_FP64 ;

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, C->type == Complex, mult->ztype == Complex))
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

    // C<Mask> = accum(C,A.*B)
    METHOD (GrB_eWiseMult (C, Mask, accum, mult, A, B, desc)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

