//------------------------------------------------------------------------------
// LAGraph_xinit:  start GraphBLAS and LAGraph
//------------------------------------------------------------------------------

/*
    LAGraph:  graph algorithms based on GraphBLAS

    Copyright 2019 LAGraph Contributors.

    (see Contributors.txt for a full list of Contributors; see
    ContributionInstructions.txt for information on how you can Contribute to
    this project).

    All Rights Reserved.

    NO WARRANTY. THIS MATERIAL IS FURNISHED ON AN "AS-IS" BASIS. THE LAGRAPH
    CONTRIBUTORS MAKE NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR IMPLIED,
    AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF FITNESS FOR
    PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF
    THE MATERIAL. THE CONTRIBUTORS DO NOT MAKE ANY WARRANTY OF ANY KIND WITH
    RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.

    Released under a BSD license, please see the LICENSE file distributed with
    this Software or contact permission@sei.cmu.edu for full terms.

    Created, in part, with funding and support from the United States
    Government.  (see Acknowledgments.txt file).

    This program includes and/or can make use of certain third party source
    code, object code, documentation and other files ("Third Party Software").
    See LICENSE file for more details.

*/

//------------------------------------------------------------------------------

// LAGraph_xinit:  start GraphBLAS and LAGraph
// Contributed by Tim Davis, Texas A&M

#include "LAGraph_internal.h"

#define LAGRAPH_FREE_ALL                    \
{                                           \
    LAGraph_free_global ( ) ;               \
}

/*
GrB_Info GxB_init           // start up GraphBLAS and also define malloc, etc
(
    GrB_Mode mode,          // blocking or non-blocking mode

    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),
    void * (* user_calloc_function  ) (size_t, size_t),
    void * (* user_realloc_function ) (void *, size_t),
    void   (* user_free_function    ) (void *),

    // This argument appears in SuiteSparse:GraphBLAS 3.0.0 but not 2.x:
    bool user_malloc_is_thread_safe
) ;
*/

GrB_Info LAGraph_xinit
(
    // pointers to memory management functions
    void * (* user_malloc_function  ) (size_t),
    void * (* user_calloc_function  ) (size_t, size_t),
    void * (* user_realloc_function ) (void *, size_t),
    void   (* user_free_function    ) (void *),

    bool user_malloc_is_thread_safe
)
{

    // initialize GraphBLAS
    GrB_Info info ;

#if defined ( GxB_SUITESPARSE_GRAPHBLAS )

    #if ( GxB_IMPLEMENTATION_MAJOR >= 3 )

    LAGRAPH_OK (GxB_init (GrB_NONBLOCKING,
        user_malloc_function,
        user_calloc_function,
        user_realloc_function,
        user_free_function,
        user_malloc_is_thread_safe)) ;

    #else

    LAGRAPH_OK (GxB_init (GrB_NONBLOCKING,
        user_malloc_function,
        user_calloc_function,
        user_realloc_function,
        user_free_function)) ;

    #endif

#else

    // GxB_init is not available.  Use GrB_init instead.
    LAGRAPH_OK (GrB_init (GrB_NONBLOCKING)) ;

#endif

    // save the memory management pointers in global LAGraph space
    LAGraph_malloc_function  = user_malloc_function ;
    LAGraph_calloc_function  = user_calloc_function ;
    LAGraph_realloc_function = user_realloc_function ;
    LAGraph_free_function    = user_free_function ;
    LAGraph_malloc_is_thread_safe = user_malloc_is_thread_safe ;

    // allocate all global objects
    LAGRAPH_OK (LAGraph_Complex_init ( )) ;
    LAGRAPH_OK (LAGraph_alloc_global ( )) ;
    return (GrB_SUCCESS) ;

}

