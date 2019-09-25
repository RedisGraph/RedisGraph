//------------------------------------------------------------------------------
// GB_mex_mxm_flops: compute flops to do C<M>=A*B or C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "[result bflops] = GB_mex_mxm (M, A, B, floplimit)"

#define FREE_ALL                            \
{                                           \
    GB_MATRIX_FREE (&A) ;                   \
    GB_MATRIX_FREE (&B) ;                   \
    GB_MATRIX_FREE (&M) ;                   \
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
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;
    GrB_Matrix M = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 2 || nargin < 3 || nargin > 4)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get M (shallow copy)
    M = GB_mx_mxArray_to_Matrix (pargin [0], "M", false, false) ;
    if (M == NULL && !mxIsEmpty (pargin [0]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("M failed") ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [1], "A", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get B (shallow copy)
    B = GB_mx_mxArray_to_Matrix (pargin [2], "B", false, true) ;
    if (B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("B failed") ;
    }

    // get floplimit
    int64_t GET_SCALAR (3, int64_t, floplimit, INT64_MAX) ;

    // allocate Bflops, if it is to be computed
    int64_t bnvec = B->nvec ;
    int64_t *Bflops = NULL ;
    if (nargout > 1)
    {
        // note the calloc of Bflops
        Bflops = mxCalloc ((bnvec+1), sizeof (int64_t)) ;
    }

    // compute the flop count
    bool result = GB_AxB_flopcount (Bflops, NULL, M, A, B, floplimit, Context) ;

    // return result to MATLAB
    pargout [0] = mxCreateDoubleScalar ((double) result) ;
    if (nargout > 1)
    {
        pargout [1] = mxCreateDoubleMatrix (1, bnvec+1, mxREAL) ;
        double *Bflops_matlab = mxGetPr (pargout [1]) ; 
        for (int64_t kk = 0 ; kk <= bnvec ; kk++)
        {
            Bflops_matlab [kk] = (double) Bflops [kk] ;
        }
        mxFree (Bflops) ;
    }

    FREE_ALL ;
}

