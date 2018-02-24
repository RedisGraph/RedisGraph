//------------------------------------------------------------------------------
// GB_mex_AxB_symoolic: compute the pattern of A*B or (A*B)'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Returns a plain MATLAB sparse matrix, not a struct

#include "GB_mex.h"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&B) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GB_mx_put_global (malloc_debug) ;   \
}

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global ( ) ;
    GrB_Matrix A = NULL ;
    GrB_Matrix B = NULL ;
    GrB_Matrix C = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 5)
    {
        mexErrMsgTxt ("Usage: C = GB_mex_AxB_symbolic "
        "(A, B, atranspose, btranspose, ctranspose)") ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get A and B
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A", false) ;
    B = GB_mx_mxArray_to_Matrix (pargin [1], "B", false) ;
    if (A == NULL || B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("failed") ;
    }

    // get the atranspose option
    GET_SCALAR (2, bool, atranspose, false) ;

    // get the btranspose option
    GET_SCALAR (3, bool, btranspose, false) ;

    // get the ctranspose option
    GET_SCALAR (4, bool, ctranspose, false) ;

    int64_t anrows = (atranspose) ? A->ncols : A->nrows ;
    int64_t ancols = (atranspose) ? A->nrows : A->ncols ;

    int64_t bnrows = (btranspose) ? B->ncols : B->nrows ;
    int64_t bncols = (btranspose) ? B->nrows : B->ncols ;

    int64_t cnrows = (ctranspose) ? bncols : anrows ;
    int64_t cncols = (ctranspose) ? anrows : bncols ;

    if (ancols != bnrows)
    {
        FREE_ALL ;
        mexErrMsgTxt ("invalid inner dimensions") ;
    }

    // create the GraphBLAS output matrix C
    METHOD (GrB_Matrix_new (&C, GrB_BOOL, cnrows, cncols)) ;

    // C = A*B or (A*B)'
    METHOD (GB_AxB_symbolic (C, NULL, A, B,
        atranspose, btranspose, ctranspose)) ;

    // allocate dummy logical values for C
    GB_MALLOC_MEMORY (C->x, C->nzmax, sizeof (bool)) ;
    if (C->x == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("out of memory") ;
    }
    bool *Cx = (bool *) C->x ;
    for (int64_t k = 0 ; k < C->nzmax ; k++)
    {
        Cx [k] = true ;
    }
    C->x_shallow = false ;

    // return C to MATLAB and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", false) ;

    FREE_ALL ;
}

