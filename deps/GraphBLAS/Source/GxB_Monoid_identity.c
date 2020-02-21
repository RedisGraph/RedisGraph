//------------------------------------------------------------------------------
// GxB_Monoid_identity: return the identity of a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Monoid_identity        // return the monoid identity
(
    void *identity,                 // returns the identity of the monoid
    GrB_Monoid monoid               // monoid to query
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Monoid_identity (&identity, monoid)") ;
    GB_RETURN_IF_NULL (identity) ;
    GB_RETURN_IF_NULL_OR_FAULTY (monoid) ;
    ASSERT_MONOID_OK (monoid, "monoid for identity", GB0) ;

    //--------------------------------------------------------------------------
    // return the identity
    //--------------------------------------------------------------------------

    memcpy (identity, monoid->identity, monoid->op->ztype->size) ;
    return (GrB_SUCCESS) ;
}

