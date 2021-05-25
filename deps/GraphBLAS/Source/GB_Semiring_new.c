//------------------------------------------------------------------------------
// GB_semiring_new: create a new semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Semiring_new            // create a semiring
(
    GrB_Semiring *semiring,         // handle of semiring to create
    GrB_Monoid add,                 // additive monoid of the semiring
    GrB_BinaryOp multiply           // multiply operator of the semiring
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (semiring != NULL) ;
    (*semiring) = NULL ;
    ASSERT (add != NULL) ;
    ASSERT (multiply != NULL) ;
    ASSERT_MONOID_OK (add, "semiring->add", GB0) ;
    ASSERT_BINARYOP_OK (multiply, "semiring->multiply", GB0) ;

    //--------------------------------------------------------------------------
    // create the semiring
    //--------------------------------------------------------------------------

    // z = multiply(x,y); type of z must match monoid z = add(z,z)
    if (multiply->ztype != add->op->ztype)
    { 
        (*semiring) = NULL ;
        return (GrB_DOMAIN_MISMATCH) ;
    }

    // allocate the semiring
    (*semiring) = GB_CALLOC (1, struct GB_Semiring_opaque) ;
    if (*semiring == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    // initialize the semiring
    GrB_Semiring s = *semiring ;
    s->magic = GB_MAGIC ;
    s->add = add ;
    s->multiply = multiply ;
    s->semiring_is_builtin = false ;

    ASSERT_SEMIRING_OK (s, "new semiring", GB0) ;
    return (GrB_SUCCESS) ;
}

