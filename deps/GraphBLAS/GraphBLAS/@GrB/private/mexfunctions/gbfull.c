//------------------------------------------------------------------------------
// gbfull: convert a GraphBLAS matrix struct into a MATLAB dense matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input may be either a GraphBLAS matrix struct or a standard MATLAB
// sparse or dense matrix.  The output is a standard MATLAB dense matrix.

#include "gb_matlab.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin >= 2 && nargin <= 4 && nargout <= 1,
        "usage: C = GrB.full (A, type, id, desc)") ;

    //--------------------------------------------------------------------------
    // get a shallow copy of the input matrix
    //--------------------------------------------------------------------------

    GrB_Matrix A = gb_get_shallow (pargin [0]) ;
    GrB_Index nrows, ncols ;
    OK (GrB_Matrix_nrows (&nrows, A)) ;
    OK (GrB_Matrix_ncols (&ncols, A)) ;

    //--------------------------------------------------------------------------
    // get the type of C
    //--------------------------------------------------------------------------

    GrB_Matrix type = gb_mxstring_to_type (pargin [1]) ;

    //--------------------------------------------------------------------------
    // get the identity scalar
    //--------------------------------------------------------------------------

    GrB_Matrix id ;
    if (nargin > 2)
    { 
        id = gb_get_shallow (pargin [2]) ;
    }
    else
    { 
        // Assume the identity is zero, of the same type as C.
        // The format does not matter, since only id (0,0) will be used.
        OK (GrB_Matrix_new (&id, type, 1, 1)) ;
    }

    //--------------------------------------------------------------------------
    // get the descriptor (kind defaults to KIND_FULL)
    //--------------------------------------------------------------------------

    base_enum_t base = BASE_DEFAULT ;
    kind_enum_t kind = KIND_FULL ;
    GxB_Format_Value fmt = GxB_NO_FORMAT ;
    GrB_Descriptor desc = NULL ;
    if (nargin > 3)
    { 
        desc = gb_mxarray_to_descriptor (pargin [nargin-1], &kind, &fmt, &base);
    }

    // A determines the format of C, unless defined by the descriptor
    fmt = gb_get_format (nrows, ncols, A, NULL, fmt) ;

    //--------------------------------------------------------------------------
    // expand the identity into a dense matrix B the same size as C
    //--------------------------------------------------------------------------

    GrB_Matrix B ;
    OK (GrB_Matrix_new (&B, type, nrows, ncols)) ;
    OK (GxB_Matrix_Option_set (B, GxB_FORMAT, fmt)) ;
    gb_matrix_assign_scalar (B, NULL, NULL, id, GrB_ALL, 0, GrB_ALL, 0, NULL,
        false) ;

    //--------------------------------------------------------------------------
    // C = first (A, B)
    //--------------------------------------------------------------------------

    GrB_Matrix C ;
    OK (GrB_Matrix_new (&C, type, nrows, ncols)) ;
    OK (GxB_Matrix_Option_set (C, GxB_FORMAT, fmt)) ;
    OK (GrB_eWiseAdd_Matrix_BinaryOp (C, NULL, NULL,
        gb_first_binop (type), A, B, NULL)) ;

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&id)) ;
    OK (GrB_Matrix_free (&B)) ;
    OK (GrB_Matrix_free (&A)) ;
    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // export C to a MATLAB dense matrix
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, kind) ;
    GB_WRAPUP ;
}

