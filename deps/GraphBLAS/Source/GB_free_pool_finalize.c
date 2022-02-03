//------------------------------------------------------------------------------
// GB_free_pool_finalize: finalize the free_pool
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

void GB_free_pool_finalize (void)
{

    //--------------------------------------------------------------------------
    // free all memory pools
    //--------------------------------------------------------------------------

    for (int k = 3 ; k < 64 ; k++)
    {
        size_t size = (((size_t) 1) << k) ;
        while (1)
        { 
            // get a block from the kth free_pool and free it
            void *p = GB_Global_free_pool_get (k) ;
            if (p == NULL) break ;
            GB_free_memory (&p, size) ;
        }
    }
}

