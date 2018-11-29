//------------------------------------------------------------------------------
// GxB_BinaryOp_ztype: return the type of z for z=f(x,y)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_BinaryOp_ztype         // return the type of z
(
    GrB_Type *ztype,                // return type of output z
    const GrB_BinaryOp binaryop     // binary operator to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_BinaryOp_ztype (&ztype, binaryop)") ;
    GB_RETURN_IF_NULL (ztype) ;
    GB_RETURN_IF_NULL_OR_FAULTY (binaryop) ;
    ASSERT_OK (GB_check (binaryop, "binaryop for ztype", GB0)) ;

    //--------------------------------------------------------------------------
    // return the ztype
    //--------------------------------------------------------------------------

    (*ztype) = binaryop->ztype ;
    return (GrB_SUCCESS) ;
}

