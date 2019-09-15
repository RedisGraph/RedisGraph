//------------------------------------------------------------------------------
// GB_mex_transpose: transpose a sparse matrix and return it to MATLAB
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum (C,A') or accum (C,A)

#include "GB_mex.h"

#define USAGE "C = GB_mex_transpose (C, M, accum, A, desc, test)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GB_MATRIX_FREE (&M) ;               \
    GrB_free (&desc) ;                  \
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
    GrB_Matrix C = NULL ;
    GrB_Matrix M = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 4 || nargin > 6)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get M (shallow copy)
    if (!mxIsChar (pargin [1]))
    {
        M = GB_mx_mxArray_to_Matrix (pargin [1], "M", false, false) ;
        if (M == NULL && !mxIsEmpty (pargin [1]))
        {
            FREE_ALL ;
            mexErrMsgTxt ("M failed") ;
        }
    }

    // get A (shallow copy)
    if (!mxIsChar (pargin [3]))
    {
        A = GB_mx_mxArray_to_Matrix (pargin [3], "A input", false, true) ;
        if (A == NULL)
        {
            FREE_ALL ;
            mexErrMsgTxt ("A failed") ;
        }
    }

    // get C (make a deep copy) and get any aliases for M and A
    #define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true, true) ;          \
    if (nargin > 5 && C != NULL) C->nvec_nonempty = -1 ;  /* for testing */    \
    if (mxIsChar (pargin [1])) M = GB_mx_alias ("M", pargin [1], "C",C, "A",A);\
    if (mxIsChar (pargin [3])) A = GB_mx_alias ("A", pargin [3], "C",C, "M",M);
    #define FREE_DEEP_COPY GB_MATRIX_FREE (&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (C->type) ;

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, C->type == Complex, A->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (4), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // just for testing
    if (nargin > 5)
    {
        if (M != NULL) M->nvec_nonempty = -1 ;
        A->nvec_nonempty = -1 ;
        C->nvec_nonempty = -1 ;
    }

    // C<M> = op(C,A') or op(C,A)
    METHOD (GrB_transpose (C, M, accum, A, desc)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

