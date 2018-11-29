//------------------------------------------------------------------------------
// GxB_Semiring_add: return the additive monoid of a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Semiring_add           // return the additive monoid of a semiring
(
    GrB_Monoid *add,                // returns additive monoid of the semiring
    const GrB_Semiring semiring     // semiring to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Semiring_add (&add, semiring)") ;
    GB_RETURN_IF_NULL (add) ;
    GB_RETURN_IF_NULL_OR_FAULTY (semiring) ;
    ASSERT_OK (GB_check (semiring, "semiring for add", GB0)) ;

    //--------------------------------------------------------------------------
    // return the ztype
    //--------------------------------------------------------------------------

    (*add) = semiring->add ;
    return (GrB_SUCCESS) ;
}

