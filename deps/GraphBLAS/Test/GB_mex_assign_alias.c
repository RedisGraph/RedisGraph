//------------------------------------------------------------------------------
// GB_mex_assign_alias: C(I,J) = accum(C(I,J),C)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define FREE_ALL                            \
{                                           \
    GB_MATRIX_FREE (&C) ;                   \
    GrB_free (&desc) ;                      \
    GB_mx_put_global (malloc_debug) ;       \
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
    GrB_Matrix C = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 2 || nargin > 5)
    {
        mexErrMsgTxt ("Usage: C = GB_mex_assign_alias (C, accum, I, J, desc)");
    }

    // get C (make a deep copy)
    #define GET_DEEP_COPY \
    { \
        C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true) ; \
    }
    #define FREE_DEEP_COPY GB_MATRIX_FREE (&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (C->type) ;

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [1], "accum",
        GB_NOP_opcode, cclass, C->type == Complex, C->type == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    GrB_Index *I, *J, ni, nj ;

    // get I
    if (!GB_mx_mxArray_to_indices (&I, PARGIN (2), &ni))
    {
        FREE_ALL ;
        mexErrMsgTxt ("I failed") ;
    }

    // get J
    if (!GB_mx_mxArray_to_indices (&J, PARGIN (3), &nj))
    {
        FREE_ALL ;
        mexErrMsgTxt ("J failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (4), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    GrB_Index nrows, ncols ;
    GrB_Matrix_nvals (&nrows, C) ;
    GrB_Matrix_nvals (&ncols, C) ;

    // C(I,J) = accum (C(I,J),C)
    METHOD (GrB_assign (C, NULL, accum, C, I, ni, J, nj, desc)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

