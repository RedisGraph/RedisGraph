//------------------------------------------------------------------------------
// GB_Type_check: print a built-in type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// There are two character strings: name passed in from the caller, and
// type->name.  The caller can use the name argument to print "the type of
// matrix A:", for example.  The internal name is the C typedef with which the
// GraphBLAS GrB_Type was created.

#include "GB.h"

GrB_Info GB_Type_check      // check a GraphBLAS Type
(
    const GrB_Type type,    // GraphBLAS type to print and check
    const char *name,       // name of the type from the caller; optional
    const GB_diagnostic pr  // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (pr > 0) printf ("GraphBLAS type: %s ", NAME) ;

    if (type == NULL)
    {
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) printf ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check object
    //--------------------------------------------------------------------------

    CHECK_MAGIC (type, "Type") ;

    switch (type->code)
    {
        case GB_BOOL_code   : if (pr > 0) printf ("bool"     ) ; break ;
        case GB_INT8_code   : if (pr > 0) printf ("int8_t"   ) ; break ;
        case GB_UINT8_code  : if (pr > 0) printf ("uint8_t"  ) ; break ;
        case GB_INT16_code  : if (pr > 0) printf ("int16_t"  ) ; break ;
        case GB_UINT16_code : if (pr > 0) printf ("uint16_t" ) ; break ;
        case GB_INT32_code  : if (pr > 0) printf ("int32_t"  ) ; break ;
        case GB_UINT32_code : if (pr > 0) printf ("uint32_t" ) ; break ;
        case GB_INT64_code  : if (pr > 0) printf ("int64_t"  ) ; break ;
        case GB_UINT64_code : if (pr > 0) printf ("uint64_t" ) ; break ;
        case GB_FP32_code   : if (pr > 0) printf ("float"    ) ; break ;
        case GB_FP64_code   : if (pr > 0) printf ("double"   ) ; break ;
        case GB_UDT_code    :
            if (pr > 0) printf ("user-defined: [%s]", type->name) ;
            break ;
        default             : if (pr > 0) printf ("unknown type\n") ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "Type code %d is unknown: %s [%s]",
                type->code, NAME, type->name))) ;
    }

    if (pr > 0) printf (" size: %zu\n", type->size) ;

    if (type->size == 0 || type->size != GB_Type_size (type->code, type->size))
    {
        if (pr > 0) printf ("Type has an invalid size\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "Type has an invalid size: %s [%s]", NAME, type->name))) ;
    }

    return (GrB_SUCCESS) ; // not REPORT_SUCCESS; may mask error in caller
}

