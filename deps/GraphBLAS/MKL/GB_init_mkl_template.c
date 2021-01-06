//------------------------------------------------------------------------------
// GB_init_mkl_template: initialize GraphBLAS for MKL
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    // set the MKL allocator functions
    //--------------------------------------------------------------------------

    #if GB_HAS_MKL_GRAPH
    i_malloc  = malloc_function ;
    i_calloc  = calloc_function ;
    i_realloc = realloc_function ;
    i_free    = free_function ;
    #endif

    //--------------------------------------------------------------------------
    // control usage of Intel MKL
    //--------------------------------------------------------------------------

    GB_Global_use_mkl_set (false) ;

