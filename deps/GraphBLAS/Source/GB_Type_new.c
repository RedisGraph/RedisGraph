//------------------------------------------------------------------------------
// GB_Type_new: create a new user-defined type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This is not used for built-in types.  Those are created statically.
// Users should not call this function directly; use GrB_Type_new instead.

#include "GB.h"

GrB_Info GB_Type_new
(
    GrB_Type *type,             // handle of user type to create
    size_t sizeof_ctype,        // size of the user type
    const char *name            // name of the type, as "sizeof (ctype)"
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GrB_Type_new (&type, sizeof (ctype))") ;
    GB_RETURN_IF_NULL (type) ;
    (*type) = NULL ;

    #if ( ! GB_HAS_VLA )

        // Microsoft Visual Studio does not support variable-length arrays
        // allocating automatically on the stack.  These arrays are used for
        // scalar values for a given type.  If VLA is not supported,
        // user-defined types can be no larger than GB_VLA_MAXSIZE.

        if (sizeof_ctype > GB_VLA_MAXSIZE)
        {
            return (GrB_INVALID_VALUE) ;
        }

    #endif

    //--------------------------------------------------------------------------
    // create the type
    //--------------------------------------------------------------------------

    // allocate the type
    (*type) = GB_CALLOC (1, struct GB_Type_opaque) ;
    if (*type == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    // initialize the type
    GrB_Type t = *type ;
    t->magic = GB_MAGIC ;
    t->size = GB_IMAX (sizeof_ctype, 1) ;
    t->code = GB_UDT_code ;     // user-defined type

    //--------------------------------------------------------------------------
    // get the name
    //--------------------------------------------------------------------------

    // if no name found, a generic name is used instead
    strncpy (t->name, "user-type", GB_LEN-1) ;

    char input2 [GB_LEN+1] ;
    char *p = NULL ;

    // look for "sizeof" in the input string
    if (name != NULL)
    { 
        strncpy (input2, name, GB_LEN) ;
        p = strstr (input2, "sizeof") ;
    }

    if (p != NULL)
    { 

        // "sizeof" appears in the input string, advance past it
        p += 6 ;

        // find leading "(" if it appears, and advance to one character past it
        char *p2 = strstr (p, "(") ;
        if (p2 != NULL) p = p2 + 1 ;

        // find trailing ")" if it appears, and delete it
        p2 = strstr (p, ")") ;
        if (p2 != NULL) *p2 = '\0' ;

        // p now contains the final name, copy it to the output name
        strncpy (t->name, p, GB_LEN-1) ;
    }

    return (GrB_SUCCESS) ;
}

