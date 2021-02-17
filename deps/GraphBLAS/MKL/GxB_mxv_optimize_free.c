//------------------------------------------------------------------------------
// GxB_mxv_optimize: optimize a matrix for matrix-vector multiply
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_mkl.h"

GrB_Info GxB_mxv_optimize_free      // analyze C for subsequent use in mxv
(
    GrB_Matrix C                    // input/output matrix
)
{
#if GB_HAS_MKL_GRAPH

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE (C, "GxB_mxv_optimize_free (C, desc)") ;
    GB_BURBLE_START ("GxB_mxv_optimize_free") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;

    //--------------------------------------------------------------------------
    // free any existing MKL version of the matrix C and its optimization
    //--------------------------------------------------------------------------

    GB_MKL_GRAPH_MATRIX_DESTROY (C->mkl) ;

    C->mkl = NULL ;
#endif
    return (GrB_SUCCESS) ;
}

