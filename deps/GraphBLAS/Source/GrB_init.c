//------------------------------------------------------------------------------
// GrB_init: initialize GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_init (or GxB_init) must called before any other GraphBLAS operation.
// GrB_finalize must be called as the last GraphBLAS operation.

// not parallel: this function does O(1) work and is already thread-safe.

#include "GB.h"

GrB_Info GrB_init           // start up GraphBLAS
(
    const GrB_Mode mode     // blocking or non-blocking mode
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_init (mode)") ;

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    return (GB_init (mode, malloc, calloc, realloc, free, Context)) ;
}

