//------------------------------------------------------------------------------
// GxB_BinaryOp_ytype: return the type of y for z=f(x,y)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_BinaryOp_ytype         // return the type of y
(
    GrB_Type *ytype,                // return type of input y
    const GrB_BinaryOp binaryop     // binary operator to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_BinaryOp_ytype (&ytype, binaryop)") ;
    RETURN_IF_NULL (ytype) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (binaryop) ;
    ASSERT_OK (GB_check (binaryop, "binaryop for ytype", 0)) ;

    //--------------------------------------------------------------------------
    // return the ytype
    //--------------------------------------------------------------------------

    (*ytype) = binaryop->ytype ;
    return (REPORT_SUCCESS) ;
}

