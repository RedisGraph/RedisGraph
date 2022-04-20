//------------------------------------------------------------------------------
// GB_Semiring_new: create a new semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_Semiring_new.h"

GrB_Info GB_Semiring_new            // create a semiring
(
    GrB_Semiring semiring,          // semiring to create
    GrB_Monoid add,                 // additive monoid of the semiring
    GrB_BinaryOp multiply           // multiply operator of the semiring
)
{


//  printf("inside GB_semiring_new\n");
    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (semiring != NULL) ;
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
//      printf("z doesn't match monoid z\n");
        return (GrB_DOMAIN_MISMATCH) ;
    }

    // initialize the semiring
    semiring->magic = GB_MAGIC ;
    semiring->add = add ;
    semiring->multiply = multiply ;

    ASSERT_SEMIRING_OK (semiring, "new semiring", GB0) ;
    return (GrB_SUCCESS) ;
}

