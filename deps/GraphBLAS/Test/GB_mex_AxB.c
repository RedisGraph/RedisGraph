//------------------------------------------------------------------------------
// GB_mex_AxB: compute C=A*B, A'*B, A*B', or A'*B'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is for testing only.  See GrB_mxm instead.  Returns a plain MATLAB
// matrix, in double.

#include "GB_mex.h"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&Aconj) ;           \
    GB_MATRIX_FREE (&B) ;               \
    GB_MATRIX_FREE (&Bconj) ;           \
    GB_MATRIX_FREE (&C) ;               \
    GB_MATRIX_FREE (&Mask) ;            \
    GrB_free (&add) ;                   \
    GrB_free (&semiring) ;              \
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
    bool ignore ;
    GrB_Matrix A = NULL, B = NULL, C = NULL, Aconj = NULL, Bconj = NULL,
        Mask = NULL ;
    GrB_Monoid add = NULL ;
    GrB_Semiring semiring = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 4)
    {
        mexErrMsgTxt
            ("Usage: C = GB_mex_AxB (A, B, atranspose, btranspose)") ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    if (mxIsComplex (pargin [0]))
    {
        // just for testing
        METHOD (Complex_finalize ()) ;
        METHOD (Complex_init ()) ;
    }

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

    // determine the dimensions
    int64_t anrows = (atranspose) ? A->ncols : A->nrows ;
    int64_t ancols = (atranspose) ? A->nrows : A->ncols ;
    int64_t bnrows = (btranspose) ? B->ncols : B->nrows ;
    int64_t bncols = (btranspose) ? B->nrows : B->ncols ;
    if (ancols != bnrows)
    {
        FREE_ALL ;
        mexErrMsgTxt ("invalid dimensions") ;
    }

    // create the GraphBLAS output matrix C
    METHOD (GrB_Matrix_new (&C, A->type, anrows, bncols)) ;

    if (A->type == Complex)
    {
        // C = A*B, complex case
        if (atranspose)
        {
            // Aconj = A
            METHOD (GrB_Matrix_new (&Aconj, Complex, A->nrows, A->ncols)) ;
            METHOD (GrB_apply (Aconj, NULL, NULL, Complex_conj, A, NULL)) ;
        }
        if (btranspose)
        {
            // Bconj = B
            METHOD (GrB_Matrix_new (&Bconj, Complex, B->nrows, B->ncols)) ;
            METHOD (GrB_apply (Bconj, NULL, NULL, Complex_conj, B, NULL)) ;
        }
        METHOD (GB_Matrix_multiply (C, NULL,
            (atranspose) ? Aconj : A,
            (btranspose) ? Bconj : B, Complex_plus_times,
            atranspose, btranspose, false, &ignore)) ;
    }
    else
    {

        // create the Semiring for regular z += x*y
        METHOD (GrB_Monoid_new (&add, GrB_PLUS_FP64, (double) 0)) ;
        METHOD (GrB_Semiring_new (&semiring, add, GrB_TIMES_FP64)) ;

        // C = A*B, A'*B, A*B', or A'*B'
        METHOD (GB_Matrix_multiply (C, NULL,
            A, B, semiring, /* GrB_PLUS_TIMES_FP64 */
            atranspose, btranspose, false, &ignore)) ;
    }

    // return C to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C AxB result", false) ;

    FREE_ALL ;
    GrB_finalize ( ) ;
}

