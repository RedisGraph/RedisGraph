//------------------------------------------------------------------------------
// GB_mex_resize: resize a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#define FREE_ALL                        \
{                                       \
    GrB_free (&C) ;                     \
    GB_mx_put_global (malloc_debug) ;   \
}

#include "GB_mex.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global ( ) ;
    GrB_Matrix C = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 1 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: C = GB_mex_resize (A, nrows_new, ncols_new)") ;
    }

    #define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true) ;
    #define FREE_DEEP_COPY GB_MATRIX_FREE (&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (C->type) ;

    // get nrows_new
    GET_SCALAR (1, int64_t, nrows_new, C->nrows) ;

    // get ncols_new
    GET_SCALAR (2, int64_t, ncols_new, C->ncols) ;

    // resize the matrix
    if (C->ncols == 1 && ncols_new == 1)
    {
        // resize C as a vector
        METHOD (GxB_resize ((GrB_Vector) C, nrows_new)) ;
    }
    else
    {
        // resize C as a matrix
        METHOD (GxB_resize (C, nrows_new, ncols_new)) ;
    }

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

