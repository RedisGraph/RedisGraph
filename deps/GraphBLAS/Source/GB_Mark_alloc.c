//------------------------------------------------------------------------------
// GB_Mark_alloc: ensure Mark workspace is large enough
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

bool GB_Mark_alloc                  // allocate Mark space
(
    int64_t Mark_required           // ensure Mark is at least this large
)
{

    int64_t currsize = GB_thread_local.Mark_size ;
    if (Mark_required > currsize)
    {
        // free the existing space
        GB_FREE_MEMORY (GB_thread_local.Mark, currsize, sizeof (int64_t)) ;
        GB_thread_local.Mark_size = 0 ;

        // calloc the new space
        int64_t newsize = Mark_required + 1 ;
        GB_CALLOC_MEMORY (GB_thread_local.Mark, newsize, sizeof (int64_t)) ;
        if (GB_thread_local.Mark == NULL)
        {
            // out of memory
            return (false) ;
        }
        GB_thread_local.Mark_size = newsize ;
        GB_thread_local.Mark_flag = 1 ;
    }

    // this function can only be called when Mark [...] < Mark_flag
    // assertion for debugging only:
    ASSERT_MARK_IS_RESET ;          // assert that Mark [...] < flag

    // success
    return (true) ;
}

