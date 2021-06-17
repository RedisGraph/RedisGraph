//------------------------------------------------------------------------------
// GxB_BinaryOp_ytype: return the type of y for z=f(x,y)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_BinaryOp_ytype         // type of y
(
    GrB_Type *ytype,                // return type of input y
    GrB_BinaryOp binaryop           // binary operator to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_BinaryOp_ytype (&ytype, binaryop)") ;
    GB_RETURN_IF_NULL (ytype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (binaryop) ;
    ASSERT_BINARYOP_OK (binaryop, "binaryop for ytype", GB0) ;

    //--------------------------------------------------------------------------
    // return the ytype
    //--------------------------------------------------------------------------

    (*ytype) = binaryop->ytype ;
    return (GrB_SUCCESS) ;
}

