//------------------------------------------------------------------------------
// gbfull: add identity values to a matrix so all entries are present
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The input may be either a GraphBLAS matrix struct or a standard MATLAB
// sparse or full matrix.  The output is a GraphBLAS matrix by default, with
// all entries present, of the given type.  Entries are filled in with the id
// value, whose default value is zero.

// If desc.kind = 'grb', or if the descriptor is not present, the output is a
// GraphBLAS full matrix.  Otherwise the output is a MATLAB full matrix
// (desc.kind = 'full').   The two other cases, desc.kind = 'sparse' and
// 'matlab' are treated as 'full'.

// Usage:
//  C = gbfull (A)
//  C = gbfull (A, type)
//  C = gbfull (A, type, id)
//  C = gbfull (A, type, id, desc)

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

    gb_usage (nargin >= 1 && nargin <= 4 && nargout <= 2,
        "usage: C = gbfull (A, type, id, desc)") ;

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

    GrB_Matrix type ;
    if (nargin > 1)
    { 
        type = gb_mxstring_to_type (pargin [1]) ;
    }
    else
    { 
        // the output type defaults to the same as the input type
        OK (GxB_Matrix_type (&type, A)) ;
    }

    //--------------------------------------------------------------------------
    // get the identity scalar
    //--------------------------------------------------------------------------

    GrB_Matrix id = NULL ;
    if (nargin > 2)
    { 
        id = gb_get_shallow (pargin [2]) ;
    }

    //--------------------------------------------------------------------------
    // get the descriptor
    //--------------------------------------------------------------------------

    base_enum_t base = BASE_DEFAULT ;
    kind_enum_t kind = KIND_GRB ;
    GxB_Format_Value fmt = GxB_NO_FORMAT ;
    int sparsity = 0 ;
    GrB_Descriptor desc = NULL ;
    if (nargin > 3)
    { 
        desc = gb_mxarray_to_descriptor (pargin [nargin-1], &kind, &fmt,
            &sparsity, &base) ;
    }
    OK (GrB_Descriptor_free (&desc)) ;

    //--------------------------------------------------------------------------
    // finalize the kind and format
    //--------------------------------------------------------------------------

    // ignore desc.kind = 'sparse' or 'matlab' and just use 'full' instead
    kind = (kind == KIND_SPARSE || kind == KIND_MATLAB) ? KIND_FULL : kind ;

    if (kind == KIND_FULL)
    {
        // MATLAB matrices are always held by column
        fmt = GxB_BY_COL ;
    }
    else
    {
        // A determines the format of C, unless defined by the descriptor
        fmt = gb_get_format (nrows, ncols, A, NULL, fmt) ;
    }

    //--------------------------------------------------------------------------
    // expand A to a full matrix
    //--------------------------------------------------------------------------

    GrB_Matrix C = gb_expand_to_full (A, type, fmt, id) ;
    OK (GrB_Matrix_free (&A)) ;

    //--------------------------------------------------------------------------
    // export C
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, kind) ;
    pargout [1] = mxCreateDoubleScalar (kind) ;
    GB_WRAPUP ;
}

