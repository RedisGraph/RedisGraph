//------------------------------------------------------------------------------
// GB_mex_mis: s=mis(A), find a maximal independent set
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// s = mis (A) ; A must be symmetric

#include "GB_mex.h"

#define USAGE "iset = GB_mex_mis (A)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
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
    GrB_Matrix A = NULL, Iset = NULL ;
    GrB_Vector iset ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin != 1)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // compute the independent set
    GrB_Info info = mis_check (&iset, A) ;
    if (info != GrB_SUCCESS)
    {
        mexErrMsgTxt ("mis failed") ;
    }

    // return iset to MATLAB
    Iset = (GrB_Matrix) iset ;
    pargout [0] = GB_mx_Matrix_to_mxArray (&Iset, "iset result", false) ;

    FREE_ALL ;
    GrB_finalize ( ) ;
}

