//------------------------------------------------------------------------------
// GB_mex_Vector_extract: MATLAB interface for w<mask> = accum (w,u(I))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define FREE_ALL                        \
{                                       \
    GrB_free (&w) ;                     \
    GrB_free (&mask) ;                  \
    GrB_free (&u) ;                     \
    GrB_free (&desc) ;                  \
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
    GrB_Vector w = NULL ;
    GrB_Vector mask = NULL ;
    GrB_Vector u = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 5 || nargin > 6)
    {
        mexErrMsgTxt ("Usage: w = GB_mex_Vector_extract "
            "(w, mask, accum, u, I, desc)");
    }

    // get w (make a deep copy)
    #define GET_DEEP_COPY \
    w = GB_mx_mxArray_to_Vector (pargin [0], "w input", true) ;
    #define FREE_DEEP_COPY GrB_free (&w) ;
    GET_DEEP_COPY ;
    if (w == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("w failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (w->type) ;

    // get mask (shallow copy)
    mask = GB_mx_mxArray_to_Vector (pargin [1], "mask", false) ;
    if (mask == NULL && !mxIsEmpty (pargin [1]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("mask failed") ;
    }

    // get u (shallow copy)
    u = GB_mx_mxArray_to_Vector (pargin [3], "A input", false) ;
    if (u == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("u failed") ;
    }

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [2], "accum",
        GB_NOP_opcode, cclass, w->type == Complex, u->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get I
    GrB_Index *I, ni ; 
    if (!GB_mx_mxArray_to_indices (&I, pargin [4], &ni))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (5), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // w<mask> = accum (w,u(I))
    METHOD (GrB_Vector_extract (w, mask, accum, u, I, ni, desc)) ;

    // return w to MATLAB as a struct and free the GraphBLAS w
    pargout [0] = GB_mx_Vector_to_mxArray (&w, "w output", true) ;

    FREE_ALL ;
}

