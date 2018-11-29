//------------------------------------------------------------------------------
// GB_mex_vxm: w'<mask> = accum(w',u'A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "w = GB_mex_vxm (w, mask, accum, semiring, u, A, desc)"

#define FREE_ALL                            \
{                                           \
    GrB_free (&w) ;                         \
    GrB_free (&u) ;                         \
    GB_MATRIX_FREE (&A) ;                   \
    GrB_free (&mask) ;                      \
    if (semiring != Complex_plus_times)     \
    {                                       \
        if (semiring != NULL)               \
        {                                   \
            GrB_free (&(semiring->add)) ;   \
        }                                   \
        GrB_free (&semiring) ;              \
    }                                       \
    GrB_free (&desc) ;                      \
    GB_mx_put_global (true, AxB_method_used) ; \
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
    GrB_Vector w = NULL ;
    GrB_Vector u = NULL ;
    GrB_Matrix A = NULL ;
    GrB_Vector mask = NULL ;
    GrB_Semiring semiring = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_Desc_Value AxB_method_used = GxB_DEFAULT ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 6 || nargin > 7)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get w' (make a deep copy)
    #define GET_DEEP_COPY \
    w = GB_mx_mxArray_to_Vector (pargin [0], "w input", true, true) ;
    #define FREE_DEEP_COPY GrB_free (&w) ;
    GET_DEEP_COPY ;
    if (w == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("w failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (w->type) ;

    // get mask (shallow copy)
    mask = GB_mx_mxArray_to_Vector (pargin [1], "mask", false, false) ;
    if (mask == NULL && !mxIsEmpty (pargin [1]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("mask failed") ;
    }

    // get u (shallow copy)
    u = GB_mx_mxArray_to_Vector (pargin [4], "u input", false, true) ;
    if (u == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("u failed") ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [5], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get semiring
    if (A->type == Complex)
    {
        // semiring input argument is ignored and may be empty
        semiring = Complex_plus_times ;
    }
    else
    {
        if (!GB_mx_mxArray_to_Semiring (&semiring, pargin [3], "semiring",
            cclass))
        {
            FREE_ALL ;
            mexErrMsgTxt ("semiring failed") ;
        }
    }

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, w->type == Complex,
        semiring->add->op->ztype == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (6), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // w'<mask> = accum(w',u'*A)
    METHOD (GrB_vxm (w, mask, accum, semiring, u, A, desc)) ;

    if (w != NULL) AxB_method_used = w->AxB_method_used ;

    // return w to MATLAB as a struct and free the GraphBLAS w
    pargout [0] = GB_mx_Vector_to_mxArray (&w, "w output", true) ;

    FREE_ALL ;
}

