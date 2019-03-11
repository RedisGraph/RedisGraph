//------------------------------------------------------------------------------
// GxB_init: initialize GraphBLAS and declare malloc/calloc/realloc/free to use
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_init (or GxB_init) must called before any other GraphBLAS operation.
// GrB_finalize must be called as the last GraphBLAS operation.  GxB_init is
// identical to GrB_init, except that it allows the user application to define
// the malloc/calloc/realloc/free functions that SuiteSparse:GraphBLAS will
// use.  The functions cannot be modified once GraphBLAS starts.

// not parallel: this function does O(1) work and is already thread-safe.

// Examples:

/*

    // to use GraphBLAS with the ANSI C11 functions (or to another library
    // linked in that replaces them): 

        #include "GraphBLAS.h"
        ...
        // either use:
        GrB_init (mode) ;
        // or use this (but not both):
        GxB_init (mode, malloc, calloc, realloc, free) ;

    // to use GraphBLAS from within a MATLAB mexFunction:

        #include "mex.h"
        #include "GraphBLAS.h"
        ...
        GxB_init (mode, mxMalloc, mxCalloc, mxRealloc, mxFree) ;

    // to use the C interface to the Intel TBB scalable allocators:

        #include "tbb/scalable_allocator.h"
        #include "GraphBLAS.h"
        ...
        GxB_init (mode, scalable_malloc, scalable_calloc, scalable_realloc,
            scalable_free) ;

*/


#include "GB.h"

GrB_Info GxB_init           // start up GraphBLAS and also define malloc, etc
(
    const GrB_Mode mode,    // blocking or non-blocking mode

    // pointers to memory management functions.  If any are NULL, use the
    // built-in ANSI C11 functions.
    void * (* user_malloc_function  ) (size_t),
    void * (* user_calloc_function  ) (size_t, size_t),
    void * (* user_realloc_function ) (void *, size_t),
    void   (* user_free_function    ) (void *)
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_init (mode, malloc, calloc, realloc, free)") ;
    GB_RETURN_IF_NULL (user_malloc_function ) ;
    GB_RETURN_IF_NULL (user_calloc_function ) ;
    GB_RETURN_IF_NULL (user_realloc_function) ;
    GB_RETURN_IF_NULL (user_free_function   ) ;

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    return (GB_init (mode, user_malloc_function, user_calloc_function,
        user_realloc_function, user_free_function, Context)) ;
}

