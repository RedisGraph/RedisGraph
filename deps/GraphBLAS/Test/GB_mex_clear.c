//------------------------------------------------------------------------------
// GB_mex_clear: clear a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// clear a matrix

#include "GB_mex.h"

#define USAGE "C = GB_mex_clear (A)"

#define FREE_ALL                        \
{                                       \
    GrB_free (&A) ;                     \
    GrB_free (&C) ;                     \
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
    GrB_Matrix A = NULL, C = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 1 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY       GrB_Matrix_dup (&C, A) ;
    #define FREE_DEEP_COPY      GrB_free (&C) ;

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }
    mxClassID aclass = GB_mx_Type_to_classID (A->type) ;

    GxB_print (A,3) ;

    // output matrix has same type as input matrix
    GrB_Type ctype = A->type ;

    // copy A into C
    GrB_Matrix_dup (&C, A) ;
    GxB_print (C,3) ;

    // clear C
    METHOD (GrB_Matrix_clear (C)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

