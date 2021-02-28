//------------------------------------------------------------------------------
// GrB_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_init (or GxB_init) must called before any other GraphBLAS operation.
// GrB_finalize must be called as the last GraphBLAS operation.

#include "GB.h"

GrB_Info GrB_init           // start up GraphBLAS
(
    GrB_Mode mode           // blocking or non-blocking mode
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_CONTEXT ("GrB_init (mode)") ;

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    // default:  use the ANSI C11 malloc memory manager, which is thread-safe 

    return (GB_init (mode, malloc, calloc, realloc, free, true, Context)) ;
}

