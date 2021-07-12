//------------------------------------------------------------------------------
// GxB_Matrix_build_Scalar: build a sparse GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GxB_Matrix_build_Scalar builds a matrix C whose values in its sparsity
// pattern are all equal to a value given by a GxB_Scalar.  Unlike the
// GrB_Matrix_build_* methods, there is no binary dup operator.  Instead, any
// duplicate indices are ignored.  The I and J arrays are of size nvals,
// just like GrB_Matrix_build_*.

#include "GB_build.h"
#define GB_FREE_ALL ;

GrB_Info GxB_Matrix_build_Scalar
(
    GrB_Matrix C,                   // matrix to build
    const GrB_Index *I,             // array of row indices of tuples
    const GrB_Index *J,             // array of column indices of tuples
    GxB_Scalar scalar,              // value for all tuples
    GrB_Index nvals                 // number of tuples
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE (C, "GxB_Matrix_build_Scalar (C, I, J, scalar, nvals)") ;
    GB_BURBLE_START ("GxB_Matrix_build_Scalar") ;
    GB_RETURN_IF_NULL_OR_FAULTY (scalar) ;
    GB_MATRIX_WAIT (scalar) ;
    if (GB_nnz ((GrB_Matrix) scalar) != 1)
    { 
        GB_ERROR (GrB_INVALID_VALUE, "Scalar value is %s", "missing") ;
    }

    //--------------------------------------------------------------------------
    // build the matrix
    //--------------------------------------------------------------------------

    GrB_Info info = GB_build (C, I, J, scalar->x, nvals, NULL,
        scalar->type, true, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}

