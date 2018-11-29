//------------------------------------------------------------------------------
// GB_mex_reduce_to_vector: c = accum(c,reduce_to_vector(A))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reduce a matrix to a vector: w<mask> = accum (w, reduce_to_vector (A))

// MATLAB interface to GrB_reduce, which relies on GrB_Matrix_reduce_BinaryOp
// and GrB_Matrix_reduce_Monoid to reduce a matrix to a vector.

#include "GB_mex.h"

#define USAGE "w = GB_mex_reduce_to_vector (w, mask, accum, reduce, A, desc)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    GrB_free (&w) ;                     \
    GrB_free (&mask) ;                  \
    GrB_free (&desc) ;                  \
    if (!reduce_is_complex)             \
    {                                   \
        GrB_free (&reduce) ;            \
    }                                   \
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
    GrB_Matrix A = NULL ;
    GrB_Vector w = NULL ;
    GrB_Vector mask = NULL ;
    GrB_Descriptor desc = NULL ;
    GrB_Monoid reduce = NULL ;
    bool reduce_is_complex = false ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 5 || nargin > 6)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get w (make a deep copy)
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

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [4], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get reduce operator; default: NOP, default class is class(w)
    GrB_BinaryOp reduceop ;
    if (!GB_mx_mxArray_to_BinaryOp (&reduceop, pargin [3], "reduceop",
        GB_NOP_opcode, cclass, A->type == Complex, A->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("reduceop failed") ;
    }

    // get the reduce monoid
    if (reduceop == Complex_plus)
    {
        reduce_is_complex = true ;
        reduce = Complex_plus_monoid ;
    }
    else if (reduceop == Complex_times)
    {
        reduce_is_complex = true ;
        reduce = Complex_times_monoid ;
    }
    else
    {
        // create the reduce monoid
        if (!GB_mx_Monoid (&reduce, reduceop, malloc_debug))
        {
            FREE_ALL ;
            mexErrMsgTxt ("reduce failed") ;
        }
    }

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, w->type == Complex, reduce_is_complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (5), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // w<mask> = accum (w, reduce_to_vector (A))
    METHOD (GrB_reduce (w, mask, accum, reduce, A, desc)) ;

    // return w to MATLAB as a struct and free the GraphBLAS w
    pargout [0] = GB_mx_Vector_to_mxArray (&w, "w output", true) ;

    FREE_ALL ;
}

