//------------------------------------------------------------------------------
// GB_entry_check: print a single entry for a built-in type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// not parallel: This function does O(1) work, but can be called for every
// entry in a matrix, to print it out in GxB_Matrix_fprint or
// GxB_Vector_fprint.  The printing to a file is fundamentally sequential, and
// also only needed for diagnostics.  For Matrix Market I/O, however, printing
// of the entire matrix could perhaps be done in parallel.

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

