//------------------------------------------------------------------------------
// GxB_UnaryOp_xtype: return the type of x for z=f(x)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_UnaryOp_xtype          // return the type of x
(
    GrB_Type *xtype,                // return type of input x
    const GrB_UnaryOp unaryop       // unary operator
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_UnaryOp_xtype (&xtype, unaryop)") ;
    RETURN_IF_NULL (xtype) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (unaryop) ;
    ASSERT_OK (GB_check (unaryop, "unaryop for xtype", 0)) ;

    //--------------------------------------------------------------------------
    // return the xtype
    //--------------------------------------------------------------------------

    (*xtype) = unaryop->xtype ;
    return (REPORT_SUCCESS) ;
}

