//------------------------------------------------------------------------------
// GB_CONTEXT_mkl_template.h: definitions for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#undef  GB_CONTEXT
#define GB_CONTEXT(where_string)                                    \
    /* construct the Context */                                     \
    GB_Context_struct Context_struct ;                              \
    GB_Context Context = &Context_struct ;                          \
    /* set Context->where so GrB_error can report it if needed */   \
    Context->where = where_string ;                                 \
    /* get the default max # of threads and default chunk size */   \
    Context->nthreads_max = GB_Global_nthreads_max_get ( ) ;        \
    Context->chunk = GB_Global_chunk_get ( ) ;                      \
    /* get the pointer to where any error will be logged */         \
    Context->logger = NULL ;                                        \
    Context->use_mkl = GB_Global_use_mkl_get ( ) ;

