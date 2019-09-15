//------------------------------------------------------------------------------
// GB_Type_check: print a built-in type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// There are two character strings: name passed in from the caller, and
// type->name.  The caller can use the name argument to print "the type of
// matrix A:", for example.  The internal name is the C typedef with which the
// GraphBLAS GrB_Type was created.

// for additional diagnostics, use:
// #define GB_DEVELOPER 1

#include "GB.h"

GrB_Info GB_Type_check      // check a GraphBLAS Type
(
    const GrB_Type type,    // GraphBLAS type to print and check
    const char *name,       // name of the type from the caller; optional
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    FILE *f,                // file for output
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBPR0 ("GraphBLAS type: ") ;
    if (pr > 0 && name != NULL) GBPR ("%s ", name) ;

    if (type == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (type, "Type") ;

    switch (type->code)
    {
        case GB_BOOL_code   : GBPR0 ("bool"     ) ; break ;
        case GB_INT8_code   : GBPR0 ("int8_t"   ) ; break ;
        case GB_UINT8_code  : GBPR0 ("uint8_t"  ) ; break ;
        case GB_INT16_code  : GBPR0 ("int16_t"  ) ; break ;
        case GB_UINT16_code : GBPR0 ("uint16_t" ) ; break ;
        case GB_INT32_code  : GBPR0 ("int32_t"  ) ; break ;
        case GB_UINT32_code : GBPR0 ("uint32_t" ) ; break ;
        case GB_INT64_code  : GBPR0 ("int64_t"  ) ; break ;
        case GB_UINT64_code : GBPR0 ("uint64_t" ) ; break ;
        case GB_FP32_code   : GBPR0 ("float"    ) ; break ;
        case GB_FP64_code   : GBPR0 ("double"   ) ; break ;
        case GB_UCT_code    :
            GBPR0 ("compile-time user-defined: [%s]", type->name) ;
            break ;
        case GB_UDT_code    :
            GBPR0 ("run-time user-defined: [%s]", type->name) ;
            break ;
        default             : GBPR0 ("unknown type\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "Type code %d is unknown: %s [%s]",
                type->code, GB_NAME, type->name))) ;
    }

    GBPR0 (" size: %zu\n", type->size) ;

    if (type->size == 0 || type->size != GB_code_size (type->code, type->size))
    { 
        GBPR0 ("Type has an invalid size\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "Type has an invalid size: %s [%s]", GB_NAME, type->name))) ;
    }

    return (GrB_SUCCESS) ;
}

