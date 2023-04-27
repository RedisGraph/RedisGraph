//------------------------------------------------------------------------------
// gb_usage: check usage and make sure GrB.init has been called
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: GPL-3.0-or-later

//------------------------------------------------------------------------------

#include "gb_interface.h"

void gb_usage       // check usage and make sure GrB.init has been called
(
    bool ok,                // if false, then usage is not correct
    const char *usage       // error message if usage is not correct
)
{

    //--------------------------------------------------------------------------
    // clear the debug memory table (for debugging only)
    //--------------------------------------------------------------------------

    GB_Global_memtable_clear ( ) ;

    //--------------------------------------------------------------------------
    // make sure GrB.init has been called
    //--------------------------------------------------------------------------

    if (!GB_Global_GrB_init_called_get ( ))
    {

        //----------------------------------------------------------------------
        // initialize GraphBLAS
        //----------------------------------------------------------------------

        OK (GxB_init (GrB_NONBLOCKING, mxMalloc, mxCalloc, mxRealloc, mxFree)) ;

        // mxMalloc, mxCalloc, mxRealloc, and mxFree are not thread safe
        GB_Global_malloc_is_thread_safe_set (false) ;

        // must use mexPrintf to print to Command Window
        OK (GxB_Global_Option_set (GxB_PRINTF, mexPrintf)) ;
        OK (GxB_Global_Option_set (GxB_FLUSH, gb_flush)) ;

        // disable the memory pool
        int64_t free_pool_limit [64] =
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
             0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } ;
        OK (GxB_Global_Option_set (GxB_MEMORY_POOL, free_pool_limit)) ;

        // built-in matrices are stored by column
        OK (GxB_Global_Option_set (GxB_FORMAT, GxB_BY_COL)) ;

        // print 1-based indices
        OK (GxB_Global_Option_set (GxB_PRINT_1BASED, true)) ;

        // for debug only
        GB_Global_abort_function_set (gb_abort) ;

        // for printing memory sizes of matrices
        GB_Global_print_mem_shallow_set (true) ;
    }

    //--------------------------------------------------------------------------
    // check usage
    //--------------------------------------------------------------------------

    if (!ok)
    {
        ERROR (usage) ;
    }

    //--------------------------------------------------------------------------
    // get test coverage
    //--------------------------------------------------------------------------

    #ifdef GBCOV
    gbcov_get ( ) ;
    #endif
}

