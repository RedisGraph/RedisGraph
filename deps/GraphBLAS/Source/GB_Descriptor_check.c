//------------------------------------------------------------------------------
// GB_Descriptor_check: check and print a Descriptor
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

//------------------------------------------------------------------------------
// dcheck: check a single descriptor field
//------------------------------------------------------------------------------

static GrB_Info dcheck
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

    if (pr > 0) GBPR ("D.%s = ", field) ;
    switch (v)
    {
        case GxB_DEFAULT       : if (pr > 0) GBPR ("default   ") ; break ;
        case GrB_SCMP          : if (pr > 0) GBPR ("scmp      ") ; break ;
        case GrB_TRAN          : if (pr > 0) GBPR ("tran      ") ; break ;
        case GrB_REPLACE       : if (pr > 0) GBPR ("replace   ") ; break ;
        case GxB_AxB_GUSTAVSON : if (pr > 0) GBPR ("Gustavson ") ; break ;
        case GxB_AxB_HEAP      : if (pr > 0) GBPR ("heap      ") ; break ;
        case GxB_AxB_DOT       : if (pr > 0) GBPR ("dot       ") ; break ;
        default                : if (pr > 0) GBPR ("unknown   ") ;
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
        if (pr > 0) GBPR (" (invalid value for this field)") ;
        info = GrB_INVALID_OBJECT ;
    }

    if (pr > 0) GBPR ("\n") ;

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

    if (pr > 0) GBPR ("\nGraphBLAS Descriptor: %s ", GB_NAME) ;

    if (D == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) GBPR ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (D, "Descriptor") ;

    if (pr > 0) GBPR ("\n") ;

    GrB_Info info [5] ;
    info [0] = dcheck (true,  "output    ", D->out,  GrB_REPLACE, pr,f,Context);
    info [1] = dcheck (true,  "mask      ", D->mask, GrB_SCMP,    pr,f,Context);
    info [2] = dcheck (true,  "input0    ", D->in0,  GrB_TRAN,    pr,f,Context);
    info [3] = dcheck (true,  "input1    ", D->in1,  GrB_TRAN,    pr,f,Context);
    info [4] = dcheck (false, "AxB_method", D->axb,  0,           pr,f,Context);

    for (int i = 0 ; i < 5 ; i++)
    { 
        if (info [i] != GrB_SUCCESS)
        {
            if (pr > 0) GBPR ("Descriptor field set to an invalid value\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "Descriptor field set to an invalid value: [%s]", GB_NAME))) ;
        }
    }

    return (GrB_SUCCESS) ;
}

