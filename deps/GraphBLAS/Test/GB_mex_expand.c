//------------------------------------------------------------------------------
// GB_mex_expand: C<M,struct> = scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"
#include "matrix.h"

#define USAGE "C = GB_mex_expand (M, scalar)"

#define FREE_ALL                        \
{                                       \
    GB_MATRIX_FREE (&M) ;               \
    GB_MATRIX_FREE (&C) ;               \
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
    GrB_Matrix C = NULL, M = NULL ;

    GB_WHERE (USAGE) ;
    if (nargin != 2 || nargout > 1)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get M (shallow copy)
    M = GB_mx_mxArray_to_Matrix (pargin [0], "M", false, false) ;
    if (M == NULL && !mxIsEmpty (pargin [0]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("M failed") ;
    }

    GrB_Index nrows, ncols ;
    GrB_Matrix_nrows (&nrows, M) ;
    GrB_Matrix_ncols (&ncols, M) ;

    // get scalar
    if (!mxIsScalar (pargin [1]))
    {
        FREE_ALL ;
        mexErrMsgTxt ("scalar failed") ;
    }

    // C<M,struct> = scalar
    if (mxIsSparse (pargin [1]))
    {
        mexErrMsgTxt ("scalar must not be sparse") ;
    }
    else if (mxIsComplex (pargin [1]))
    {
        // complex case
        double xcomplex [2] ;
        GB_mx_complex_merge (1, xcomplex, pargin [1]) ;
        GrB_Matrix_new (&C, Complex, nrows, ncols) ;
        GxB_Matrix_subassign_UDT (C, M, NULL, (void *) xcomplex,
            GrB_ALL, nrows, GrB_ALL, ncols, GrB_DESC_RS) ;
    }
    else
    {
        // built-in GraphBLAS types

        #define CREATE(grb_type,c_type)                             \
        {                                                           \
            GrB_Matrix_new (&C, grb_type, nrows, ncols) ;           \
            c_type *scalar = (c_type *) mxGetData (pargin [1]) ;    \
            GxB_subassign (C, M, NULL, *scalar,                     \
                GrB_ALL, nrows, GrB_ALL, ncols, GrB_DESC_RS) ;      \
        }                                                           \
        break ;

        switch (mxGetClassID (pargin [1]))
        {
            case mxLOGICAL_CLASS : CREATE (GrB_BOOL   , bool     ) ;
            case mxDOUBLE_CLASS  : CREATE (GrB_FP64   , double   ) ;
            case mxSINGLE_CLASS  : CREATE (GrB_FP32   , float    ) ;
            case mxINT8_CLASS    : CREATE (GrB_INT8   , int8_t   ) ;
            case mxUINT8_CLASS   : CREATE (GrB_UINT8  , uint8_t  ) ;
            case mxINT16_CLASS   : CREATE (GrB_INT16  , int16_t  ) ;
            case mxUINT16_CLASS  : CREATE (GrB_UINT16 , uint16_t ) ;
            case mxINT32_CLASS   : CREATE (GrB_INT32  , int32_t  ) ;
            case mxUINT32_CLASS  : CREATE (GrB_UINT32 , uint32_t ) ;
            case mxINT64_CLASS   : CREATE (GrB_INT64  , int64_t  ) ;
            case mxUINT64_CLASS  : CREATE (GrB_UINT64 , uint64_t ) ;
            default: mexErrMsgTxt ("scalar type not supported") ;
        }
    }

    // return result to MATLAB
    pargout [0] = GB_mx_Matrix_to_mxArray (&C, "C result", true) ;
    FREE_ALL ;
}

