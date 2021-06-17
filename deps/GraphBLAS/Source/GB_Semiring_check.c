//------------------------------------------------------------------------------
// GB_Semiring_check: check and print a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Semiring_check          // check a GraphBLAS semiring
(
    const GrB_Semiring semiring,    // GraphBLAS semiring to print and check
    const char *name,               // name of the semiring, optional
    int pr,                         // print level
    FILE *f                         // file for output
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBPR0 ("\n    GraphBLAS Semiring: %s ", GB_NAME) ;

    if (semiring == NULL)
    { 
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (semiring, "Semiring") ;
    GBPR0 (semiring->semiring_is_builtin ? "(built-in)" : "(user-defined)") ;

    GrB_Info info ;
    info = GB_Monoid_check (semiring->add, "semiring->add", pr, f) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("    Semiring->add invalid\n") ;
        return (GrB_INVALID_OBJECT) ;
    }

    info = GB_BinaryOp_check (semiring->multiply, "semiring->multiply", pr, f) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("    Semiring->multiply invalid\n") ;
        return (GrB_INVALID_OBJECT) ;
    }

    // z = multiply(x,y); type of z must match monoid type
    if (semiring->multiply->ztype != semiring->add->op->ztype)
    { 
        GBPR0 ("    Semiring multiply output domain must match monoid"
            " domain\n") ;
        return (GrB_INVALID_OBJECT) ;
    }

    return (GrB_SUCCESS) ;
}

