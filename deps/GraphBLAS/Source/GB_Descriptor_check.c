//------------------------------------------------------------------------------
// GB_Descriptor_check: check and print a Descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

//------------------------------------------------------------------------------
// dcheck: check a single descriptor field
//------------------------------------------------------------------------------

static void dcheck
(
    GrB_Info *info,
    const char *field,
    const GrB_Desc_Value v,
    const GrB_Desc_Value nondefault,
    const GB_diagnostic pr
)
{

    if (pr > 0) printf ("D.%s = ", field) ;
    switch (v)
    {
        case GxB_DEFAULT: if (pr > 0) printf ("default") ; break ;
        case GrB_SCMP:    if (pr > 0) printf ("scmp   ") ; break ;
        case GrB_TRAN:    if (pr > 0) printf ("tran   ") ; break ;
        case GrB_REPLACE: if (pr > 0) printf ("replace") ; break ;
        default:          if (pr > 0) printf ("unknown") ;
            *info = GrB_INVALID_OBJECT ;
            break ;
    }

    // descriptor field can be set to the default, or one non-default value
    if (! (v == GxB_DEFAULT || v == nondefault))
    {
        if (pr > 0) printf (" (invalid value for this field)") ;
        *info = GrB_INVALID_OBJECT ;
    }

    if (pr > 0) printf ("\n") ;
}

//------------------------------------------------------------------------------
// GB_Descriptor_check
//------------------------------------------------------------------------------

GrB_Info GB_Descriptor_check    // check a GraphBLAS descriptor
(
    const GrB_Descriptor D,     // GraphBLAS descriptor to print and check
    const char *name,           // name of the descriptor, optional
    const GB_diagnostic pr      // 0: print nothing, 1: print header and
                                // errors, 2: print brief, 3: print all
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (pr > 0) printf ("\nGraphBLAS Descriptor: %s ", NAME) ;

    if (D == NULL)
    {
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) printf ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    CHECK_MAGIC (D, "Descriptor") ;

    if (pr > 0) printf ("\n") ;

    GrB_Info info = GrB_SUCCESS ;
    dcheck (&info, "output", D->out,  GrB_REPLACE, pr) ;
    dcheck (&info, "mask",   D->mask, GrB_SCMP,    pr) ;
    dcheck (&info, "input0", D->in0,  GrB_TRAN,    pr) ;
    dcheck (&info, "input1", D->in1,  GrB_TRAN,    pr) ;

    if (info != GrB_SUCCESS)
    {
        if (pr > 0) printf ("Descriptor field set to an invalid value\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "Descriptor field set to an invalid value: [%s]", NAME))) ;
    }

    return (GrB_SUCCESS) ; // not REPORT_SUCCESS; may mask error in caller
}

