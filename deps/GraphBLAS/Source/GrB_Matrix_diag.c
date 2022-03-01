//------------------------------------------------------------------------------
// GrB_Matrix_diag: construct a diagonal matrix from a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Identical to GxB_Matrix_diag (C, v, k, NULL)

#include "GB_diag.h"

GrB_Info GrB_Matrix_diag    // construct a diagonal matrix from a vector
(
    GrB_Matrix C,                   // output matrix
    const GrB_Vector v,             // input vector
    int64_t k
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE (C, "GrB_Matrix_diag (C, v, k)") ;
    GB_BURBLE_START ("GrB_Matrix_diag") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;

    //--------------------------------------------------------------------------
    // C = diag (v,0)
    //--------------------------------------------------------------------------

    GrB_Info info = GB_Matrix_diag (C, (GrB_Matrix) v, k, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}

