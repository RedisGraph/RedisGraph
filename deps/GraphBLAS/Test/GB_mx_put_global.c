//------------------------------------------------------------------------------
// GB_mx_put_global: put the GraphBLAS status in global workspace
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mex.h"

void GB_mx_put_global
(
    bool cover
)
{

    //--------------------------------------------------------------------------
    // free the complex type and operators
    //--------------------------------------------------------------------------

    Complex_finalize ( ) ;

    //--------------------------------------------------------------------------
    // log the statement coverage
    //--------------------------------------------------------------------------

    #ifdef GBCOVER
    if (cover) GB_cover_put ( ) ;
    #endif

    //--------------------------------------------------------------------------
    // finalize GraphBLAS
    //--------------------------------------------------------------------------

    GrB_finalize ( ) ;

    // disable the memory pool, in case a @GrB method is called next
    int64_t free_pool_limit [64] =
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } ;
    GxB_Global_Option_set (GxB_MEMORY_POOL, free_pool_limit) ;

    //--------------------------------------------------------------------------
    // check nmemtable and nmalloc
    //--------------------------------------------------------------------------

    int nmemtable = GB_Global_memtable_n ( ) ;
    if (nmemtable != 0)
    {
        printf ("in GB_mx_put_global: GraphBLAS nmemtable %d!\n", nmemtable) ;
        GB_Global_free_pool_dump (2) ;
        GB_Global_memtable_dump ( ) ;
        mexErrMsgTxt ("memory leak in test!") ;
    }

    int64_t nblocks = GB_Global_free_pool_nblocks_total ( ) ;
    if (nblocks != 0)
    {
        printf ("in GB_mx_put_global: GraphBLAS nblocks "GBd" in free_pool!\n",
            nblocks) ;
        GB_Global_free_pool_dump (2) ;
        GB_Global_memtable_dump ( ) ;
        mexErrMsgTxt ("memory leak in test!") ;
    }

    int64_t nmalloc = GB_Global_nmalloc_get ( ) ;
    if (nmalloc != 0)
    {
        printf ("in GB_mx_put_global: GraphBLAS nmalloc "GBd"!\n", nmalloc) ;
        GB_Global_free_pool_dump (2) ;
        GB_Global_memtable_dump ( ) ;
        mexErrMsgTxt ("memory leak in test!") ;
    }

    //--------------------------------------------------------------------------
    // allow GrB_init to be called again
    //--------------------------------------------------------------------------

    GB_Global_GrB_init_called_set (false) ;
}

