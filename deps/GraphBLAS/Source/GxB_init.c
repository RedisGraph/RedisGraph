//------------------------------------------------------------------------------
// GxB_init: initialize GraphBLAS and declare malloc/calloc/realloc/free to use
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GrB_init (or GxB_init) must called before any other GraphBLAS operation.
// GrB_finalize must be called as the last GraphBLAS operation.  GxB_init is
// identical to GrB_init, except that it allows the user application to define
// the malloc/calloc/realloc/free functions that SuiteSparse:GraphBLAS will
// use.  The functions cannot be modified once GraphBLAS starts.

// Examples:

/*

    // to use GraphBLAS with the ANSI C11 functions (or to another library
    // linked in that replaces them): 

        // either use:
        GrB_init (mode) ;
        // or use this (but not both):
        GxB_init (mode, malloc, calloc, realloc, free, true) ;

    // to use GraphBLAS from within a MATLAB mexFunction:

        #include "mex.h"
        GxB_init (mode, mxMalloc, mxCalloc, mxRealloc, mxFree, false) ;

    // to use the C interface to the Intel TBB scalable allocators:

        #include "tbb/scalable_allocator.h"
        GxB_init (mode, scalable_malloc, scalable_calloc, scalable_realloc,
            scalable_free, true) ;

*/


#include "GB.h"

GrB_Info GxB_init           // start up GraphBLAS and also define malloc, etc
(
    GrB_Mode mode,          // blocking or non-blocking mode

    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),
    void * (* user_calloc_function  ) (size_t, size_t),
    void * (* user_realloc_function ) (void *, size_t),
    void   (* user_free_function    ) (void *),
    bool user_malloc_is_thread_safe
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_CONTEXT ("GxB_init (mode, malloc, calloc, realloc, free, thread_safe)") ;
    if (user_malloc_function == NULL || user_realloc_function == NULL ||
        user_calloc_function == NULL || user_free_function == NULL)
    { 
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // initialize GraphBLAS
    //--------------------------------------------------------------------------

    return (GB_init (mode, user_malloc_function, user_calloc_function,
        user_realloc_function, user_free_function, user_malloc_is_thread_safe,
        Context)) ;
}

