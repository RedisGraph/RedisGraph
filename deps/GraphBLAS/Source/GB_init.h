//------------------------------------------------------------------------------
// GB_init.h: definitions for GB_init
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_INIT_H
#define GB_INIT_H

GrB_Info GB_init            // start up GraphBLAS
(
    const GrB_Mode mode,    // blocking or non-blocking mode

    // pointers to memory management functions
    void * (* malloc_function  ) (size_t),
    void * (* realloc_function ) (void *, size_t),
    void   (* free_function    ) (void *),

    GB_Context Context      // from GrB_init or GxB_init
) ;

#endif

