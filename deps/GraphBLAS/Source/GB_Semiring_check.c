//------------------------------------------------------------------------------
// GB_Semiring_check: check and print a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Semiring_check          // check a GraphBLAS semiring
(
    const GrB_Semiring semiring,    // GraphBLAS semiring to print and check
    const char *name,               // name of the semiring, optional
    const GB_diagnostic pr          // 0: print nothing, 1: print header and
                                    // errors, 2: print brief, 3: print all
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (pr > 0) printf ("\nGraphBLAS Semiring: %s ", NAME) ;

    if (semiring == NULL)
    {
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) printf ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    CHECK_MAGIC (semiring, "Semiring") ;

    if (pr > 0)
        printf ((semiring->user_defined) ? "user-defined\n" : "built-in\n") ;

    GrB_Info info ;
    info = GB_Monoid_check (semiring->add, "semiring->add", pr) ;
    if (info != GrB_SUCCESS)
    {
        if (pr > 0) printf ("Semiring->add invalid\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "Semiring->add is an invalid monoid: [%s]", NAME))) ;
    }

    info = GB_BinaryOp_check (semiring->multiply, "semiring->multiply", pr) ;
    if (info != GrB_SUCCESS)
    {
        if (pr > 0) printf ("Semiring->multiply invalid\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "Semiring->multiply is an invalid operator: [%s]", NAME))) ;
    }

    // z = multiply(x,y); type of z must match monoid type
    if (semiring->multiply->ztype != semiring->add->op->ztype)
    {
        if (pr > 0) printf ("Semiring multiply output domain must match"
            "monoid domain\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "Semiring multiply output domain must match monoid domain: [%s]",
            NAME))) ;
    }

    return (GrB_SUCCESS) ; // not REPORT_SUCCESS; may mask error in caller
}

