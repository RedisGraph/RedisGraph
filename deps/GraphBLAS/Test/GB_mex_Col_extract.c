//------------------------------------------------------------------------------
// GB_mex_Col_extract: MATLAB interface for w<mask> = accum (w,A(I,j))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "w = GB_mex_Col_extract (w, mask, accum, A, I, j, desc)"

#define FREE_ALL                        \
{                                       \
    GrB_free (&w) ;                     \
    GrB_free (&mask) ;                  \
    GB_MATRIX_FREE (&A) ;               \
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
    GrB_Vector w = NULL ;
    GrB_Vector mask = NULL ;
    GrB_Matrix A = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_Index *I = NULL, ni = 0, I_range [3] ;
    GrB_Index *J = NULL, nj = 0, J_range [3] ;
    bool ignore ;

    GB_WHERE (USAGE) ;

    // check inputs
    if (nargout > 1 || nargin < 6 || nargin > 7)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get w (make a deep copy)
    #define GET_DEEP_COPY \
    w = GB_mx_mxArray_to_Vector (pargin [0], "w input", true, true) ;
    #define FREE_DEEP_COPY GrB_free (&w) ;
    GET_DEEP_COPY ;
    if (w == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("w failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (w->type) ;

    // get mask (shallow copy)
    mask = GB_mx_mxArray_to_Vector (pargin [1], "mask", false, false) ;
    if (mask == NULL && !mxIsEmpty (pargin [1]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("mask failed") ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [3], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, w->type == Complex, A->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get I
    if (!GB_mx_mxArray_to_indices (&I, pargin [4], &ni, I_range, &ignore))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I failed") ;
    }

    // get J
    if (!GB_mx_mxArray_to_indices (&J, pargin [5], &nj, J_range, &ignore))
    {
        FREE_ALL ;
        mexErrMsgTxt ("J failed") ;
    }

    if (nj != 1)
    {
        FREE_ALL ;
        mexErrMsgTxt ("j must be a scalar") ;
    }

    GrB_Index j = J [0] ;

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (6), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // w<mask> = accum (w,A(I,j))
    METHOD (GrB_Col_extract (w, mask, accum, A, I, ni, j, desc)) ;

    // return w to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Vector_to_mxArray (&w, "w output", true) ;

    FREE_ALL ;
}

