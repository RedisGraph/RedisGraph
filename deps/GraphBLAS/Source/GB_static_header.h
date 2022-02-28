//------------------------------------------------------------------------------
// GB_static_header.h: macros for allocating static headers
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GBNSTATIC
    #if defined (GBCUDA)
    #define GBNSTATIC 1
    #else
    #define GBNSTATIC 0
    #endif
#endif

#undef GB_CLEAR_STATIC_HEADER

#if GBNSTATIC

    // do not use any static headers
    #define GB_CLEAR_STATIC_HEADER(XX,XX_header_handle)                     \
    {                                                                       \
        size_t XX_size ;                                                    \
        XX = GB_CALLOC (1, struct GB_Matrix_opaque, &XX_size) ;             \
        if (XX == NULL)                                                     \
        {                                                                   \
            GB_FREE_ALL ;                                                   \
            return (GrB_OUT_OF_MEMORY) ;                                    \
        }                                                                   \
        XX->static_header = false ;                                         \
        XX->header_size = XX_size ;                                         \
        XX->magic = GB_MAGIC2 ;                                             \
    }

#else

    // use static headers
    #define GB_CLEAR_STATIC_HEADER(XX,XX_header_handle)                     \
    {                                                                       \
        XX = GB_clear_static_header (XX_header_handle) ;                    \
    }

#endif

