//------------------------------------------------------------------------------
// GB_mex_resize: resize a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "C = GB_mex_resize (A, nrows_new, ncols_new)"

#define FREE_ALL                        \
{                                       \
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
    GrB_Matrix C = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 1 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

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

    // get vlen_new
    int64_t GET_SCALAR (1, int64_t, vlen_new, C->vlen) ;

    // get vdim_new
    int64_t GET_SCALAR (2, int64_t, vdim_new, C->vdim) ;

    // resize the matrix
    if (GB_VECTOR_OK (C) && vdim_new == 1)
    {
        // resize C as a vector
        METHOD (GxB_resize ((GrB_Vector) C, vlen_new)) ;
    }
    else
    {
        // resize C as a matrix
        METHOD (GxB_resize (C, vlen_new, vdim_new)) ;
    }

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

