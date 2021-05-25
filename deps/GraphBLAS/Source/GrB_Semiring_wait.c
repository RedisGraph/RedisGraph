//------------------------------------------------------------------------------
// GrB_Semiring_wait: wait for a user-defined GrB_Semiring to complete
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// In SuiteSparse:GraphBLAS, a user-defined GrB_Semiring has no pending
// operations to wait for.  All this method does is verify that the semiring is
// properly initialized.

#include "GB.h"

GrB_Info GrB_Semiring_wait   // no work, just check if the GrB_Semiring is valid
(
    GrB_Semiring *semiring
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GrB_Semiring_wait (&semiring)") ;
    GB_RETURN_IF_NULL (semiring) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*semiring) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

