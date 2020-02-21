//------------------------------------------------------------------------------
// GB_mex_mxm_flops: compute flops to do C=A*B, C<M>=A*B or C<!M>=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "[bflops mwork] = GB_mex_mxm_flops (M, Mask_comp, A, B)"

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
    if (nargout > 2 || nargin != 4)
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

    // get Mask_comp
    bool GET_SCALAR (1, bool, Mask_comp, 0) ;
    // printf ("complement: %d\n", Mask_comp) ;

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [2], "A", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get B (shallow copy)
    B = GB_mx_mxArray_to_Matrix (pargin [3], "B", false, true) ;
    if (B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("B failed") ;
    }

    // allocate Bflops (note the calloc)
    int64_t bnvec = B->nvec ;
    int64_t *Bflops = mxCalloc ((bnvec+1), sizeof (int64_t)) ;

    // compute the flop count
    int64_t Mwork = 0 ;

    GB_AxB_flopcount (&Mwork, Bflops, M, Mask_comp, A, B, Context) ;

    // return result to MATLAB
    pargout [0] = mxCreateDoubleMatrix (1, bnvec+1, mxREAL) ;
    double *Bflops_matlab = mxGetPr (pargout [0]) ; 
    for (int64_t kk = 0 ; kk <= bnvec ; kk++)
    {
        Bflops_matlab [kk] = (double) Bflops [kk] ;
    }

    pargout [1] = mxCreateDoubleScalar (Mwork) ;
    mxFree (Bflops) ;
    FREE_ALL ;
}

