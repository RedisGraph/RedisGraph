//------------------------------------------------------------------------------
// GB_mex_eWiseAdd_Matrix: C<M> = accum(C,A+B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE \
    "C = GB_mex_eWiseAdd_Matrix (C, M, accum, add, A, B, desc, test)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&B) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GrB_free (&desc) ;                  \
    GB_MATRIX_FREE (&M) ;               \
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
    GrB_Matrix M = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 6 || nargin > 8)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get C (make a deep copy)
    #define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;   \
    if (nargin > 7 && C != NULL) C->nvec_nonempty = -1 ;
    #define FREE_DEEP_COPY GB_MATRIX_FREE (&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (C->type) ;

    // get M (shallow copy)
    M = GB_mx_mxArray_to_Matrix (pargin [1], "M", false, false) ;
    if (M == NULL && !mxIsEmpty (pargin [1]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("M failed") ;
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

    // get add operator
    GrB_BinaryOp add ;
    if (!GB_mx_mxArray_to_BinaryOp (&add, pargin [3], "add",
        GB_NOP_opcode, cclass, A->type == Complex, B->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("add failed") ;
    }

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, C->type == Complex, add->ztype == Complex))
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

    // just for testing
    if (nargin > 7)
    {
        if (M != NULL) M->nvec_nonempty = -1 ;
        A->nvec_nonempty = -1 ;
        B->nvec_nonempty = -1 ;
        C->nvec_nonempty = -1 ;
    }

//      GB_check (A, "A for ewiseAdd", GB0) ;
//      GB_check (B, "B for ewiseAdd", GB0) ;
//      printf ("nvec_nonempty: %g %g\n",
//          (double) A->nvec_nonempty,
//          (double) B->nvec_nonempty) ;
//      if (M != NULL)
//      {
//          GB_check (B, "B for ewiseAdd", GB0) ;
//          printf ("M->nvec_nonempty: %g \n", (double) M->nvec_nonempty) ;
//      }

    // C<M> = accum(C,A+B)
    METHOD (GrB_eWiseAdd (C, M, accum, add, A, B, desc)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

