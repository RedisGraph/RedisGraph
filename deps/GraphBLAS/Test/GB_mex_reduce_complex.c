//------------------------------------------------------------------------------
// GB_mex_mxm: C<Mask> = accum(C,A*B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// c = prod (A) with terminal times monoid (terminal value is 0)

#include "GB_mex.h"

#define USAGE "c = GB_mex_reduce_complex (A, hack)"

#define FREE_ALL                            \
{                                           \
    GB_MATRIX_FREE (&A) ;                   \
    GrB_free (&Times_terminal) ;            \
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
    GrB_Info info ;
    GrB_Matrix A = NULL ;
    GrB_Monoid Times_terminal = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 1 || nargin > 2)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get A (shallow copy; must be complex)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }
    if (A->type != Complex)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A must be complex") ;
    }

    double complex one  = CMPLX(1,0) ;
    double complex zero = CMPLX(0,0) ;

    // create the monoid
    info = GxB_Monoid_terminal_new_UDT (&Times_terminal, Complex_times,
        &one, &zero) ;
    if (info != GrB_SUCCESS)
    {
        FREE_ALL ;
        mexErrMsgTxt ("Times_terminal failed") ;
    }

    // GxB_print (Times_terminal, 3) ;

    int64_t GET_SCALAR (1, int64_t, hack, -1) ;
    if (hack >= 0)
    {
        double complex *Ax = A->x ;
        Ax [hack] = 0 ;
        // GxB_print (A, 2) ;
    }

    // reduce to a scalar
    double complex c = zero ;
    info = GrB_Matrix_reduce_UDT (&c, NULL, Times_terminal, A, NULL) ;
    if (info != GrB_SUCCESS)
    {
        FREE_ALL ;
        mexErrMsgTxt ("reduce failed") ;
    }

    // return C to MATLAB as a scalar
    pargout [0] = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxCOMPLEX) ;
    GB_mx_complex_split (1, (double *) (&c), pargout [0]) ;

    FREE_ALL ;
}

