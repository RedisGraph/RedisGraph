//------------------------------------------------------------------------------
// GxB_Monoid_identity: return the identity of a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Monoid_identity        // return the monoid identity
(
    void *identity,                 // returns the identity of the monoid
    const GrB_Monoid monoid         // monoid to query
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Monoid_identity (&identity, monoid)") ;
    RETURN_IF_NULL (identity) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (monoid) ;
    ASSERT_OK (GB_check (monoid, "monoid for idenitity", 0)) ;

    //--------------------------------------------------------------------------
    // return the identity
    //--------------------------------------------------------------------------

    memcpy (identity, monoid->identity, monoid->op->ztype->size) ;
    return (REPORT_SUCCESS) ;
}

