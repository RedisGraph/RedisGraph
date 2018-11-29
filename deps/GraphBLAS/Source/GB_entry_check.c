//------------------------------------------------------------------------------
// GB_entry_check: print a single entry for a built-in type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_entry_check     // print a single value
(
    const GrB_Type type,    // type of value to print
    const void *x,          // value to print
    FILE *f,                // file to print to
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_RETURN_IF_NULL (x) ;
    GB_RETURN_IF_NULL_OR_FAULTY (type) ;

    //--------------------------------------------------------------------------
    // print the value
    //--------------------------------------------------------------------------

    return (GB_code_check (type->code, x, f, Context)) ;
}

