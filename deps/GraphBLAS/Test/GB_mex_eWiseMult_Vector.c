//------------------------------------------------------------------------------
// GB_mex_eWiseMult_Vector: w<mask> = accum(w,u.*v)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define USAGE "w = GB_mex_eWiseMult_Vector (w, mask, accum, mult, u, v, desc)"

#define FREE_ALL            \
{                           \
    GrB_free (&w) ;         \
    GrB_free (&u) ;         \
    GrB_free (&v) ;         \
    GrB_free (&desc) ;      \
    GrB_free (&mask) ;      \
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
    GrB_Vector w = NULL ;
    GrB_Vector u = NULL ;
    GrB_Vector v = NULL ;
    GrB_Vector mask = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin < 6 || nargin > 7)
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

    // get u (shallow copy)
    u = GB_mx_mxArray_to_Vector (pargin [4], "u input", false, true) ;
    if (u == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("u failed") ;
    }

    // get v (shallow copy)
    v = GB_mx_mxArray_to_Vector (pargin [5], "v input", false, true) ;
    if (v == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("v failed") ;
    }

    // get mult operator
    GrB_BinaryOp mult ;
    if (!GB_mx_mxArray_to_BinaryOp (&mult, pargin [3], "mult",
        GB_NOP_opcode, cclass, u->type == Complex, v->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("mult failed") ;
    }

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, w->type == Complex, mult->ztype == Complex))
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

    // w<mask> = accum(w,u.*v)
    METHOD (GrB_eWiseMult (w, mask, accum, mult, u, v, desc)) ;

    // return w to MATLAB as a struct and free the GraphBLAS w
    pargout [0] = GB_mx_Vector_to_mxArray (&w, "w output", true) ;

    FREE_ALL ;
}

