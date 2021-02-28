//------------------------------------------------------------------------------
// GxB_Monoid_operator: return the op of a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Monoid_operator        // return the monoid operator
(
    GrB_BinaryOp *op,               // returns the binary op of the monoid
    GrB_Monoid monoid               // monoid to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Monoid_operator (&op, monoid)") ;
    GB_RETURN_IF_NULL (op) ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;
    ASSERT_MONOID_OK (monoid, "monoid for op", GB0) ;

    //--------------------------------------------------------------------------
    // return the ztype
    //--------------------------------------------------------------------------

    (*op) = monoid->op ;
    return (GrB_SUCCESS) ;
}

