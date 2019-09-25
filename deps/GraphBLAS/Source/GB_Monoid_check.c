//------------------------------------------------------------------------------
// GB_Monoid_check: check and print a monoid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// for additional diagnostics, use:
// #define GB_DEVELOPER 1

#include "GB.h"

GrB_Info GB_Monoid_check        // check a GraphBLAS monoid
(
    const GrB_Monoid monoid,    // GraphBLAS monoid to print and check
    const char *name,           // name of the monoid, optional
    int pr,                     // 0: print nothing, 1: print header and errors,
                                // 2: print brief, 3: print all
    FILE *f,                    // file for output
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBPR0 ("\nGraphBLAS Monoid: %s ", GB_NAME) ;

    if (monoid == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (monoid, "Monoid") ;

    switch (monoid->object_kind)
    {
        case GB_BUILTIN :
            GBPR0 ("(built-in)") ;
            break ;

        case GB_USER_COMPILED :
            GBPR0 ("(user-defined at compile-time)") ;
            break ;

        case GB_USER_RUNTIME :
            GBPR0 ("(user-defined at run-time)") ;
            break ;

        default :
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "Monoid->object_kind is invalid: [%s]", GB_NAME))) ;
    }

    GrB_Info info = GB_BinaryOp_check (monoid->op, "monoid->op", pr, f,
        Context) ;
    if (info != GrB_SUCCESS)
    { 
        GBPR0 ("Monoid contains an invalid operator\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "Monoid contains an invalid operator: [%s]", GB_NAME))) ;
    }

    if (monoid->op->xtype != monoid->op->ztype ||
        monoid->op->ytype != monoid->op->ztype)
    { 
        GBPR0 ("All domains of operator must be the same\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "All domains of monoid operator must be the same: [%s]",
            GB_NAME))) ;
    }

    // print the identity value
    if (pr > 0)
    { 
        GBPR ("identity: [ ") ;
        info = GB_entry_check (monoid->op->ztype, monoid->identity, f,  
            Context) ;
        if (info != GrB_SUCCESS) return (info) ;
        GBPR (" ] ") ;
        // print the terminal value, if present
        if (monoid->terminal != NULL)
        { 
            GBPR ("terminal: [ ") ;
            info = GB_entry_check (monoid->op->ztype, monoid->terminal, f,  
                Context) ;
            if (info != GrB_SUCCESS) return (info) ;
            GBPR (" ]") ;
        }
        GBPR ("\n") ;
    }

    return (GrB_SUCCESS) ;
}

