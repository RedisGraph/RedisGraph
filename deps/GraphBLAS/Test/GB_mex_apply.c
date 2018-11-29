//------------------------------------------------------------------------------
// GB_mex_apply: C<Mask> = accum(C,op(A)) or op(A')
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Apply a unary operator to a matrix

#include "GB_mex.h"

#define USAGE "C = GB_mex_apply (C, Mask, accum, op, A, desc)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&C) ;               \
    GB_MATRIX_FREE (&Mask) ;            \
    GB_MATRIX_FREE (&A) ;               \
    GrB_free (&desc) ;                  \
    GB_mx_put_global (true, 0) ;        \
}

GrB_Matrix C = NULL ;
GrB_Matrix Mask = NULL ;
GrB_Matrix A = NULL ;
GrB_Descriptor desc = NULL ;
GrB_BinaryOp accum = NULL ;
GrB_UnaryOp op = NULL ;

//------------------------------------------------------------------------------

GrB_Info apply (bool is_matrix)
{
    GrB_Info info ;

    if (is_matrix)
    {
        info = GrB_apply (C, Mask, accum, op, A, desc) ;
    }
    else
    {
        info = GrB_apply ((GrB_Vector) C, (GrB_Vector) Mask, accum, op,
            (GrB_Vector) A, desc) ;
    }

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

    bool malloc_debug = GB_mx_get_global (true) ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 5 || nargin > 6)
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
    if (A == NULL || A->magic != GB_MAGIC)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get accum; default: NOP, default class is class(C)
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, C->type == Complex, A->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get op; default: NOP, default class is class(C)
    if (!GB_mx_mxArray_to_UnaryOp (&op, pargin [3], "op",
        GB_NOP_opcode, cclass, A->type == Complex)) 
    {
        FREE_ALL ;
        mexErrMsgTxt ("UnaryOp failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (5), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // C<Mask> = accum(C,op(A))
    if (GB_NCOLS (C) == 1 && (desc == NULL || desc->in0 == GxB_DEFAULT))
    {
        // this is just to test the Vector version
        METHOD (apply (false)) ;
    }
    else
    {
        METHOD (apply (true)) ;
    }

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

