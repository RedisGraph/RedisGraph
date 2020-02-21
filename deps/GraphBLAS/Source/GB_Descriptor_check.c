//------------------------------------------------------------------------------
// GB_Descriptor_check: check and print a Descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_printf.h"

//------------------------------------------------------------------------------
// GB_dc: check a single descriptor field
//------------------------------------------------------------------------------

static GrB_Info GB_dc
(
    int kind,                           // 0, 1, or 2
    const char *field,
    const GrB_Desc_Value v,
    const GrB_Desc_Value nondefault,    // for kind == 0
    int pr,
    FILE *f,
    GB_Context Context
)
{

    bool ok = true ;
    GrB_Info info = GrB_SUCCESS ;

    GBPR0 ("    d.%s = ", field) ;
    switch ((int) v)
    {
        case GxB_DEFAULT            : GBPR0 ("default   ") ; break ;
        case GrB_COMP               : GBPR0 ("complement") ; break ;
        case GrB_STRUCTURE          : GBPR0 ("structure ") ; break ;
        case GrB_COMP+GrB_STRUCTURE : GBPR0 ("structural complement") ; break ;
        case GrB_TRAN               : GBPR0 ("transpose ") ; break ;
        case GrB_REPLACE            : GBPR0 ("replace   ") ; break ;
        case GxB_AxB_SAXPY          : GBPR0 ("saxpy     ") ; break ;
        case GxB_AxB_GUSTAVSON      : GBPR0 ("Gustavson ") ; break ;
        case GxB_AxB_HEAP           : GBPR0 ("heap      ") ; break ;
        case GxB_AxB_HASH           : GBPR0 ("hash      ") ; break ;
        case GxB_AxB_DOT            : GBPR0 ("dot       ") ; break ;
        default                     : GBPR0 ("unknown   ") ;
            info = GrB_INVALID_OBJECT ;
            ok = false ;
            break ;
    }

    if (ok)
    {
        if (kind == 0)
        {
            // descriptor field can be set to the default,
            // or one non-default value
            if (! (v == GxB_DEFAULT || v == nondefault))
            { 
                ok = false ;
            }
        }
        else if (kind == 1)
        {
            // mask
            if (! (v == GxB_DEFAULT || v == GrB_COMP || v == GrB_STRUCTURE ||
                   v == (GrB_COMP + GrB_STRUCTURE)))
            {
                ok = false ;
            }
        }
        else // kind == 2
        {
            // GxB_AxB_METHOD:
            if (! (v == GxB_DEFAULT || v == GxB_AxB_GUSTAVSON
                || v == GxB_AxB_HEAP || v == GxB_AxB_DOT
                || v == GxB_AxB_HASH || v == GxB_AxB_SAXPY))
            { 
                ok = false ;
            }
        }
    }

    if (!ok)
    { 
        GBPR0 (" (invalid value for this field)") ;
        info = GrB_INVALID_OBJECT ;
    }

    GBPR0 ("\n") ;

    return (info) ;
}

//------------------------------------------------------------------------------
// GB_Descriptor_check
//------------------------------------------------------------------------------

GrB_Info GB_Descriptor_check    // check a GraphBLAS descriptor
(
    const GrB_Descriptor D,     // GraphBLAS descriptor to print and check
    const char *name,           // name of the descriptor, optional
    int pr,                     // 0: print nothing, 1: print header and
                                // errors, 2: print brief, 3: print all
    FILE *f,                    // file for output
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBPR0 ("\n    GraphBLAS Descriptor: %s ", GB_NAME) ;

    if (D == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (D, "Descriptor") ;

    GBPR0 ("\n") ;

    GrB_Info info [5] ;
    info [0] = GB_dc (0, "out     ", D->out,  GrB_REPLACE, pr,f,Context) ;
    info [1] = GB_dc (1, "mask    ", D->mask, 0,           pr,f,Context) ;
    info [2] = GB_dc (0, "in0     ", D->in0,  GrB_TRAN,    pr,f,Context) ;
    info [3] = GB_dc (0, "in1     ", D->in1,  GrB_TRAN,    pr,f,Context) ;
    info [4] = GB_dc (2, "axb     ", D->axb,  0,           pr,f,Context) ;

    for (int i = 0 ; i < 5 ; i++)
    {
        if (info [i] != GrB_SUCCESS)
        { 
            GBPR0 ("    Descriptor field set to an invalid value\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "Descriptor field set to an invalid value: [%s]", GB_NAME))) ;
        }
    }

    int nthreads_max = D->nthreads_max ;
    double chunk = D->chunk ;

    if (pr > 0)
    {
        GBPR ("    d.nthreads = ") ;
        if (nthreads_max <= GxB_DEFAULT)
        { 
            GBPR ("default\n") ;
        }
        else
        { 
            GBPR ("%d\n", nthreads_max) ;
        }
        GBPR ("    d.chunk    = ") ;
        if (chunk <= GxB_DEFAULT)
        { 
            GBPR ("default\n") ;
        }
        else
        { 
            GBPR ("%g\n", chunk) ;
        }
    }

    return (GrB_SUCCESS) ;
}

