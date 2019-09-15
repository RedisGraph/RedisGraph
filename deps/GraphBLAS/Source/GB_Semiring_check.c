//------------------------------------------------------------------------------
// GB_Semiring_check: check and print a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// for additional diagnostics, use:
// #define GB_DEVELOPER 1

#include "GB.h"

GrB_Info GB_Semiring_check          // check a GraphBLAS semiring
(
    const GrB_Semiring semiring,    // GraphBLAS semiring to print and check
    const char *name,               // name of the semiring, optional
    int pr,                         // 0: print nothing, 1: print header and
                                    // errors, 2: print brief, 3: print all
    FILE *f,                        // file for output
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBPR0 ("\nGraphBLAS Semiring: %s ", GB_NAME) ;

    if (semiring == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (semiring, "Semiring") ;

    switch (semiring->object_kind)
    {
        case GB_BUILTIN:
            GBPR0 ("(built-in)") ;
            break ;

        case GB_USER_COMPILED:
            GBPR0 ("(user-defined at compile-time)") ;
            break ;

        case GB_USER_RUNTIME:
            GBPR0 ("(user-defined at run-time)") ;
            break ;

        default:
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "Semiring->object_kind is invalid: [%s]", GB_NAME))) ;
    }

    GrB_Info info ;
    info = GB_Monoid_check (semiring->add, "semiring->add", pr, f, Context) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("Semiring->add invalid\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "Semiring->add is an invalid monoid: [%s]", GB_NAME))) ;
    }

    info = GB_BinaryOp_check (semiring->multiply, "semiring->multiply", pr, f,
        Context) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("Semiring->multiply invalid\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "Semiring->multiply is an invalid operator: [%s]", GB_NAME))) ;
    }

    // z = multiply(x,y); type of z must match monoid type
    if (semiring->multiply->ztype != semiring->add->op->ztype)
    { 
        GBPR0 ("Semiring multiply output domain must match monoid domain\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "Semiring multiply output domain must match monoid domain: [%s]",
            GB_NAME))) ;
    }

    return (GrB_SUCCESS) ;
}

