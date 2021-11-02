//------------------------------------------------------------------------------
// LAGraph_Xinit: start GraphBLAS and LAGraph, and set malloc/etc functions
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
// Contributed by Tim Davis, Texas A&M University.

//------------------------------------------------------------------------------

#include "LG_internal.h"

int LAGraph_Xinit           // returns 0 if successful, -1 if failure
(
    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),
    void * (* user_calloc_function  ) (size_t, size_t),
    void * (* user_realloc_function ) (void *, size_t),
    void   (* user_free_function    ) (void *),
    char *msg
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // malloc and free are required; realloc is optional
    LG_CLEAR_MSG ;
    LG_CHECK (user_malloc_function == NULL, -1, "malloc is NULL") ;
    LG_CHECK (user_free_function   == NULL, -1, "free is NULL") ;

    //--------------------------------------------------------------------------
    // start GraphBLAS
    //--------------------------------------------------------------------------

    #if LG_SUITESPARSE

        #if ( GxB_IMPLEMENTATION >= GxB_VERSION (5,0,2) )
        // calloc may be NULL
        #else
        // calloc is required
        LG_CHECK (user_calloc_function == NULL, -1, "calloc is NULL") ;
        #endif

        GrB_TRY (GxB_init (GrB_NONBLOCKING,
            user_malloc_function,
            user_calloc_function,
            user_realloc_function,
            user_free_function
            #if (GxB_IMPLEMENTATION_MAJOR <= 5)
            , true
            #endif
            )) ;

    #else

        // GxB_init is not available.  Use GrB_init instead.
        GrB_TRY (GrB_init (GrB_NONBLOCKING)) ;

    #endif

    //--------------------------------------------------------------------------
    // save the memory management pointers in global LAGraph space
    //--------------------------------------------------------------------------

    LAGraph_Malloc_function  = user_malloc_function ;
    LAGraph_Calloc_function  = user_calloc_function ;
    LAGraph_Realloc_function = user_realloc_function ;
    LAGraph_Free_function    = user_free_function ;

    return (0) ;
}

