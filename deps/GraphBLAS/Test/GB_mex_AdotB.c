//------------------------------------------------------------------------------
// GB_mex_AdotB: compute C=spones(Mask).*(A'*B)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Returns a plain MATLAB sparse matrix, not a struct.  Only works in double.

#include "GB_mex.h"

#define USAGE "C = GB_mex_AdotB (A,B,Mask)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GB_MATRIX_FREE (&Aconj) ;           \
    GB_MATRIX_FREE (&B) ;               \
    GB_MATRIX_FREE (&C) ;               \
    GB_MATRIX_FREE (&Mask) ;            \
    GrB_free (&add) ;                   \
    GrB_free (&semiring) ;              \
    GB_mx_put_global (true, GxB_AxB_DOT) ; \
}

GrB_Matrix A = NULL, B = NULL, C = NULL, Aconj = NULL, Mask = NULL ;
GrB_Monoid add = NULL ;
GrB_Semiring semiring = NULL ;

//------------------------------------------------------------------------------

GrB_Info adotb_complex (GB_Context Context)
{
    GrB_Info info = GrB_Matrix_new (&Aconj, Complex, A->vlen, A->vdim) ;
    if (info != GrB_SUCCESS) return (info) ;
    info = GrB_apply (Aconj, NULL, NULL, Complex_conj, A, NULL) ;
    if (info != GrB_SUCCESS)
    {
        GrB_free (&Aconj) ;
        return (info) ;
    }

    #ifdef MY_COMPLEX
    // use the precompiled complex type
    if (Aconj != NULL) Aconj->type = My_Complex ;
    if (B     != NULL) B->type     = My_Complex ;
    #endif

    info = GB_AxB_dot (&C, Mask, Aconj, B,
        #ifdef MY_COMPLEX
            My_Complex_plus_times,
        #else
            Complex_plus_times,
        #endif
        false, Context) ;

    #ifdef MY_COMPLEX
    // convert back to run-time complex type
    if (C     != NULL) C->type     = Complex ;
    if (B     != NULL) B->type     = Complex ;
    if (Aconj != NULL) Aconj->type = Complex ;
    #endif

    GrB_free (&Aconj) ;
    return (info) ;
}

//------------------------------------------------------------------------------

GrB_Info adotb (GB_Context Context) 
{
    // create the Semiring for regular z += x*y
    GrB_Info info = GrB_Monoid_new (&add, GrB_PLUS_FP64, (double) 0) ;
    if (info != GrB_SUCCESS) return (info) ;
    info = GrB_Semiring_new (&semiring, add, GrB_TIMES_FP64) ;
    if (info != GrB_SUCCESS)
    {
        GrB_free (&add) ;
        return (info) ;
    }
    // C = A'*B
    info = GB_AxB_dot (&C, Mask, A, B,
        semiring /* GxB_PLUS_TIMES_FP64 */, false, Context) ;
    GrB_free (&add) ;
    GrB_free (&semiring) ;
    return (info) ;
}

//------------------------------------------------------------------------------

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    bool malloc_debug = GB_mx_get_global (true) ;

    GB_WHERE (USAGE) ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 3)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    GET_DEEP_COPY ;
    // get A and B (shallow copies)
    A = GB_mx_mxArray_to_Matrix (pargin [0], "A input", false, true) ;
    B = GB_mx_mxArray_to_Matrix (pargin [1], "B input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }
    if (B == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("B failed") ;
    }

    // get Mask (shallow copy)
    if (nargin > 2)
    {
        Mask = GB_mx_mxArray_to_Matrix (pargin [2], "Mask input", false, false);
    }

    if (A->vlen != B->vlen)
    {
        FREE_ALL ;
        mexErrMsgTxt ("inner dimensions of A'*B do not match") ;
    }

    if (A->type == Complex)
    {
        // C = A'*B, complex case
        METHOD (adotb_complex (Context)) ;
    }
    else
    {
        METHOD (adotb (Context)) ;
    }

    // return C to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C AdotB result", false) ;

    FREE_ALL ;
}

