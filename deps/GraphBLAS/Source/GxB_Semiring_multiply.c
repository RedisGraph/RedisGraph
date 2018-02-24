//------------------------------------------------------------------------------
// GxB_Semiring_multiply: return the multiply operator of a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Semiring_multiply      // return multiply operator of a semiring
(
    GrB_BinaryOp *multiply,         // returns multiply operator of the semiring
    const GrB_Semiring semiring     // semiring to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Semiring_multiply (&multiply, semiring)") ;
    RETURN_IF_NULL (multiply) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (semiring) ;
    ASSERT_OK (GB_check (semiring, "semiring for mult", 0)) ;

    //--------------------------------------------------------------------------
    // return the ztype
    //--------------------------------------------------------------------------

    (*multiply) = semiring->multiply ;
    return (REPORT_SUCCESS) ;
}

