//------------------------------------------------------------------------------
// GB_Type_new: create a new user-defined type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This is not used for built-in types.  Those are created statically.
// Users should not call this function directly; use GrB_Type_new instead,
// which is a macro #define'd in GraphBLAS.h.

#include "GB.h"

GrB_Info GB_Type_new            // create a new GraphBLAS type
(
    GrB_Type *type,             // handle of user type to create
    const size_t size,          // size of the user type
    const char *name            // name of the type
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Type_new (&type, <type>)") ;
    RETURN_IF_NULL (type) ;
    (*type) = NULL ;

    //--------------------------------------------------------------------------
    // create the type
    //--------------------------------------------------------------------------

    // allocate the type
    GB_CALLOC_MEMORY (*type, 1, sizeof (GB_Type_opaque)) ;
    if (*type == NULL)
    {
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG, "out of memory"))) ;
    }

    // initialize the type
    GrB_Type t = *type ;
    t->magic = MAGIC ;
    t->size = size ;
    t->code = GB_UDT_code ;
    strncpy (t->name, name, GB_LEN-1) ;
    return (REPORT_SUCCESS) ;
}

