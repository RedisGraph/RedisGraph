//------------------------------------------------------------------------------
// GB_Monoid_check: check and print a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Monoid_check        // check a GraphBLAS monoid
(
    const GrB_Monoid monoid,    // GraphBLAS monoid to print and check
    const char *name,           // name of the monoid, optional
    const GB_diagnostic pr      // 0: print nothing, 1: print header and errors,
                                // 2: print brief, 3: print all
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (pr > 0) printf ("\nGraphBLAS Monoid: %s ", NAME) ;

    if (monoid == NULL)
    {
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) printf ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    CHECK_MAGIC (monoid, "Monoid") ;

    GrB_Info info = GB_BinaryOp_check (monoid->op, "monoid->op", pr) ;
    if (info != GrB_SUCCESS)
    {
        if (pr > 0) printf ("Monoid contains an invalid operator\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "Monoid contains an invalid operator: [%s]", NAME))) ;
    }

    if (monoid->op->xtype != monoid->op->ztype ||
        monoid->op->ytype != monoid->op->ztype)
    {
        if (pr > 0) printf ("All domains of operator must be the same\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "All domains of monoid operator must be the same: [%s]", NAME))) ;
    }

    // print the identity value
    if (pr > 0)
    {
        printf ("identity: [ ") ;
        if (monoid->identity_is_zero)
        {
            printf ("zero") ;
        }
        else
        {
            if (monoid->op->ztype->code != GB_UDT_code)
            {
                GB_Entry_print (monoid->op->ztype, monoid->identity) ;
            }
            else
            {
                printf ("a user-defined nonzero value") ;
            }
        }
        printf (" ]\n") ;
    }

    return (GrB_SUCCESS) ; // not REPORT_SUCCESS; may mask error in caller
}

