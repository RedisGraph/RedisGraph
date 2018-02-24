//------------------------------------------------------------------------------
// GxB_UnaryOp_ztype: return the type of z for z=f(x)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_UnaryOp_ztype          // return the type of z
(
    GrB_Type *ztype,                // return type of output z
    const GrB_UnaryOp unaryop       // unary operator
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_UnaryOp_ztype (&ztype, unaryop)") ;
    RETURN_IF_NULL (ztype) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (unaryop) ;
    ASSERT_OK (GB_check (unaryop, "unaryop for ztype", 0)) ;

    //--------------------------------------------------------------------------
    // return the ztype
    //--------------------------------------------------------------------------

    (*ztype) = unaryop->ztype ;
    return (REPORT_SUCCESS) ;
}

