//------------------------------------------------------------------------------
// GB_mex_mxm_alias: C<C> = accum(C,C*C)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define FREE_ALL                            \
{                                           \
    GB_MATRIX_FREE (&C) ;                   \
    if (semiring != Complex_plus_times)     \
    {                                       \
        if (semiring != NULL)               \
        {                                   \
            GrB_free (&(semiring->add)) ;   \
        }                                   \
        GrB_free (&semiring) ;              \
    }                                       \
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
    GrB_Semiring semiring = NULL ;
    GrB_Descriptor desc = NULL ;

    // check inputs
    if (nargout > 1 || nargin < 3 || nargin > 4)
    {
        mexErrMsgTxt ("Usage: C = GB_mex_mxm_alias (C, accum, semiring, desc)");
    }

    // get C (make a deep copy)
    #define GET_DEEP_COPY \
    C = GB_mx_mxArray_to_Matrix (pargin [0], "C input", true) ;
    #define FREE_DEEP_COPY GB_MATRIX_FREE (&C) ;
    GET_DEEP_COPY ;
    if (C == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("C failed") ;
    }
    mxClassID cclass = GB_mx_Type_to_classID (C->type) ;

    // get semiring
    if (C->type == Complex)
    {
        // semiring input argument is ignored and may be empty
        semiring = Complex_plus_times ;
    }
    else
    {
        if (!GB_mx_mxArray_to_Semiring (&semiring, pargin [2], "semiring",
            cclass))
        {
            FREE_ALL ;
            mexErrMsgTxt ("semiring failed") ;
        }
    }

    // get accum; default: NOP, default class is class(C)
    GrB_BinaryOp accum ;
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [1], "accum",
        GB_NOP_opcode, cclass, C->type == Complex,
        semiring->add->op->ztype == Complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    // get desc
    if (!GB_mx_mxArray_to_Descriptor (&desc, PARGIN (3), "desc"))
    {
        FREE_ALL ;
        mexErrMsgTxt ("desc failed") ;
    }

    // C<C> = accum(C,C*C)
    METHOD (GrB_mxm (C, C, accum, semiring, C, C, desc)) ;

    // return C to MATLAB as a struct and free the GraphBLAS C
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C output", true) ;

    FREE_ALL ;
}

