//------------------------------------------------------------------------------
// GB_mex_reduce_to_scalar: c = accum(c,reduce_to_scalar(A))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reduce a matrix or vector to a scalar

#include "GB_mex.h"

#define USAGE "c = GB_mex_reduce_to_scalar (c, accum, reduce, A)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&A) ;               \
    if (!reduce_is_complex)             \
    {                                   \
        GrB_free (&reduce) ;            \
    }                                   \
    if (ctype == Complex)               \
    {                                   \
        GB_FREE_MEMORY (c, 1, sizeof (double complex)) ; \
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
    GrB_Monoid reduce = NULL ;
    bool reduce_is_complex = false ;

    // check inputs
    GB_WHERE (USAGE) ;
    if (nargout > 1 || nargin != 4)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    #define GET_DEEP_COPY ;
    #define FREE_DEEP_COPY ;

    // get the scalar c
    void *c ;
    int64_t cnrows, cncols ;
    mxClassID cclass ;
    GrB_Type ctype ;

    GB_mx_mxArray_to_array (pargin [0], &c, &cnrows, &cncols, &cclass,
        &ctype) ;
    if (cnrows != 1 || cncols != 1)
    {
        mexErrMsgTxt ("c must be a scalar") ;
    }
    if (ctype == NULL)
    {
        mexErrMsgTxt ("c must be numeric") ;
    }

    // get A (shallow copy)
    A = GB_mx_mxArray_to_Matrix (pargin [3], "A input", false, true) ;
    if (A == NULL)
    {
        FREE_ALL ;
        mexErrMsgTxt ("A failed") ;
    }

    // get reduce; default: NOP, default class is class(C)
    GrB_BinaryOp reduceop ;
    if (!GB_mx_mxArray_to_BinaryOp (&reduceop, pargin [2], "reduceop",
        GB_NOP_opcode, cclass, ctype == Complex, ctype == Complex))
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
    if (!GB_mx_mxArray_to_BinaryOp (&accum, pargin [1], "accum",
        GB_NOP_opcode, cclass, ctype == Complex, reduce_is_complex))
    {
        FREE_ALL ;
        mexErrMsgTxt ("accum failed") ;
    }

    GrB_Descriptor d = NULL ;

    // c = accum(C,A*B)

    // test both Vector and Matrix methods.  The typecast is not necessary,
    // just to test.

    #define REDUCE(type,X) \
        METHOD (GrB_reduce ((type *) c, accum, reduce, X, d)) ;

    if (A->type == Complex)
    {
        if (A->vdim == 1)
        {
            GrB_Vector V ;
            V = (GrB_Vector) A ;
            REDUCE (void, V) ;
        }
        else
        {
            REDUCE (void, A) ;
        }
    }
    else
    {
        if (A->vdim == 1)
        {
            GrB_Vector V ;
            V = (GrB_Vector) A ;
            switch (cclass)
            {
                // all GraphBLAS built-in types are supported
                case mxLOGICAL_CLASS : REDUCE (bool     , V) ; break ;
                case mxINT8_CLASS    : REDUCE (int8_t   , V) ; break ;
                case mxUINT8_CLASS   : REDUCE (uint8_t  , V) ; break ;
                case mxINT16_CLASS   : REDUCE (int16_t  , V) ; break ;
                case mxUINT16_CLASS  : REDUCE (uint16_t , V) ; break ;
                case mxINT32_CLASS   : REDUCE (int32_t  , V) ; break ;
                case mxUINT32_CLASS  : REDUCE (uint32_t , V) ; break ;
                case mxINT64_CLASS   : REDUCE (int64_t  , V) ; break ;
                case mxUINT64_CLASS  : REDUCE (uint64_t , V) ; break ;
                case mxSINGLE_CLASS  : REDUCE (float    , V) ; break ;
                case mxDOUBLE_CLASS  : REDUCE (double   , V) ; break ;
                case mxCELL_CLASS    :
                case mxCHAR_CLASS    :
                case mxUNKNOWN_CLASS :
                case mxFUNCTION_CLASS:
                case mxSTRUCT_CLASS  :
                default              :
                    FREE_ALL ;
                    mexErrMsgTxt ("unsupported class") ;
            }
        }
        else
        {
            switch (cclass)
            {
                // all GraphBLAS built-in types are supported
                case mxLOGICAL_CLASS : REDUCE (bool     , A) ; break ;
                case mxINT8_CLASS    : REDUCE (int8_t   , A) ; break ;
                case mxUINT8_CLASS   : REDUCE (uint8_t  , A) ; break ;
                case mxINT16_CLASS   : REDUCE (int16_t  , A) ; break ;
                case mxUINT16_CLASS  : REDUCE (uint16_t , A) ; break ;
                case mxINT32_CLASS   : REDUCE (int32_t  , A) ; break ;
                case mxUINT32_CLASS  : REDUCE (uint32_t , A) ; break ;
                case mxINT64_CLASS   : REDUCE (int64_t  , A) ; break ;
                case mxUINT64_CLASS  : REDUCE (uint64_t , A) ; break ;
                case mxSINGLE_CLASS  : REDUCE (float    , A) ; break ;
                case mxDOUBLE_CLASS  : REDUCE (double   , A) ; break ;
                case mxCELL_CLASS    :
                case mxCHAR_CLASS    :
                case mxUNKNOWN_CLASS :
                case mxFUNCTION_CLASS:
                case mxSTRUCT_CLASS  :
                default              :
                    FREE_ALL ;
                    mexErrMsgTxt ("unsupported class") ;
            }
        }
    }

    // return C to MATLAB as a scalar
    if (ctype == Complex)
    {
        pargout [0] = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxCOMPLEX) ;
        GB_mx_complex_split (1, c, pargout [0]) ;
    }
    else
    {
        pargout [0] = mxCreateNumericMatrix (1, 1, cclass, mxREAL) ;
        void *p = mxGetData (pargout [0]) ;
        memcpy (p, c, ctype->size) ;
    }

    FREE_ALL ;
}

