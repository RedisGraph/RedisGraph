//------------------------------------------------------------------------------
// GB_Descriptor_check: check and print a Descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// for additional diagnostics, use:
// #define GB_DEVELOPER 1

#include "GB.h"

//------------------------------------------------------------------------------
// GB_dc: check a single descriptor field
//------------------------------------------------------------------------------

static GrB_Info GB_dc
(
    bool spec,
    const char *field,
    const GrB_Desc_Value v,
    const GrB_Desc_Value nondefault,
    int pr,
    FILE *f,
    GB_Context Context
)
{

    bool ok = true ;
    GrB_Info info = GrB_SUCCESS ;

    GBPR0 ("D.%s = ", field) ;
    switch (v)
    {
        case GxB_DEFAULT       : GBPR0 ("default   ") ; break ;
        case GrB_SCMP          : GBPR0 ("scmp      ") ; break ;
        case GrB_TRAN          : GBPR0 ("tran      ") ; break ;
        case GrB_REPLACE       : GBPR0 ("replace   ") ; break ;
        case GxB_AxB_GUSTAVSON : GBPR0 ("Gustavson ") ; break ;
        case GxB_AxB_HEAP      : GBPR0 ("heap      ") ; break ;
        case GxB_AxB_DOT       : GBPR0 ("dot       ") ; break ;
        default                : GBPR0 ("unknown   ") ;
            info = GrB_INVALID_OBJECT ;
            ok = false ;
            break ;
    }

    if (ok)
    {
        if (spec)
        {
            // descriptor field can be set to the default,
            // or one non-default value
            if (! (v == GxB_DEFAULT || v == nondefault))
            { 
                ok = false ;
            }
        }
        else
        {
            // GxB_AxB_METHOD:
            if (! (v == GxB_DEFAULT || v == GxB_AxB_GUSTAVSON
                || v == GxB_AxB_HEAP || v == GxB_AxB_DOT))
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

    GBPR0 ("\nGraphBLAS Descriptor: %s ", GB_NAME) ;

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
    info [0] = GB_dc (true,  "output    ", D->out,  GrB_REPLACE, pr,f,Context) ;
    info [1] = GB_dc (true,  "mask      ", D->mask, GrB_SCMP,    pr,f,Context) ;
    info [2] = GB_dc (true,  "input0    ", D->in0,  GrB_TRAN,    pr,f,Context) ;
    info [3] = GB_dc (true,  "input1    ", D->in1,  GrB_TRAN,    pr,f,Context) ;
    info [4] = GB_dc (false, "AxB_method", D->axb,  GxB_DEFAULT, pr,f,Context) ;

    for (int i = 0 ; i < 5 ; i++)
    {
        if (info [i] != GrB_SUCCESS)
        { 
            GBPR0 ("Descriptor field set to an invalid value\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "Descriptor field set to an invalid value: [%s]", GB_NAME))) ;
        }
    }

    int nthreads_max = D->nthreads_max ;
    double chunk = D->chunk ;

    if (pr > 0)
    {
        GBPR ("D.nthreads = ") ;
        if (nthreads_max <= GxB_DEFAULT)
        { 
            GBPR ("default\n") ;
        }
        else
        { 
            GBPR ("%d\n", nthreads_max) ;
        }
        GBPR ("D.chunk = ") ;
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

