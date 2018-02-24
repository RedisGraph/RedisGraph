//------------------------------------------------------------------------------
// GxB_Monoid_operator: return the op of a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Monoid_operator        // return the monoid operator
(
    GrB_BinaryOp *op,               // returns the binary op of the monoid
    const GrB_Monoid monoid         // monoid to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Monoid_operator (&op, monoid)") ;
    RETURN_IF_NULL (op) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (monoid) ;
    ASSERT_OK (GB_check (monoid, "monoid for op", 0)) ;

    //--------------------------------------------------------------------------
    // return the ztype
    //--------------------------------------------------------------------------

    (*op) = monoid->op ;
    return (REPORT_SUCCESS) ;
}

