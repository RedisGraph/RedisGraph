//------------------------------------------------------------------------------
// GB_Entry_print: print a single entry for a built-in type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_Entry_print     // print a single value
(
    const GrB_Type type,    // type of value to print
    const void *x           // value to print
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    RETURN_IF_NULL (x) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (type) ;

    //--------------------------------------------------------------------------
    // print the value
    //--------------------------------------------------------------------------

    GB_code_print (type->code, x) ;

    return (REPORT_SUCCESS) ;
}

