//------------------------------------------------------------------------------
// GxB_Matrix_diag: construct a diagonal matrix from a vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_diag.h"

GrB_Info GxB_Matrix_diag    // construct a diagonal matrix from a vector
(
    GrB_Matrix C,                   // output matrix
    const GrB_Vector v,             // input vector
    int64_t k,
    const GrB_Descriptor desc       // unused, except threading control
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE (C, "GxB_Matrix_diag (C, v, k, desc)") ;
    GB_BURBLE_START ("GxB_Matrix_diag") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_NULL_OR_FAULTY (v) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;

    //--------------------------------------------------------------------------
    // C = diag (v,k)
    //--------------------------------------------------------------------------

    info = GB_Matrix_diag (C, (GrB_Matrix) v, k, Context) ;
    GB_BURBLE_END ;
    return (info) ;
}

