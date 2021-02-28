//------------------------------------------------------------------------------
// GB_mex_wathen: construct a random finite-element matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "A = GB_mex_wathen (nx, ny, method, scale, rho)"

#define FREE_ALL GB_mx_put_global (true, 0) ;

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

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin > 5)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get nx
    int64_t GET_SCALAR (0, int64_t, nx, 4) ;

    // get ny
    int64_t GET_SCALAR (1, int64_t, ny, 4) ;

    // get method
    int GET_SCALAR (2, int, method, 0) ;

    // get scale
    bool GET_SCALAR (3, bool, scale, false) ;

    // get rho
    double *rho = NULL ;
    if (nargin > 4)
    {
        if (mxGetM (pargin [4]) != nx || mxGetN (pargin [4]) != ny)
        {
            mexErrMsgTxt ("rho has wrong size") ;
        }
        if (mxGetClassID (pargin [4]) != mxDOUBLE_CLASS ||
            mxIsSparse (pargin [4]))
        {
            mexErrMsgTxt ("rho must be a dense and double") ;
        }
        rho = mxGetData (pargin [4]) ;
    }

    // construct the Wathen matrix
    simple_rand_seed (1) ;
    GrB_Info info = wathen (&A, nx, ny, scale, method, rho) ;
    if (info != GrB_SUCCESS)
    {
        mexErrMsgTxt ("wathen failed") ;
    }

    // return A to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&A, "A final", false) ;

    FREE_ALL ;
}

