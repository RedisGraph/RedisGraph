//------------------------------------------------------------------------------
// GB_Mark_reset: increment the Mark workspace flag
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

int64_t GB_Mark_reset
(
    int64_t reset,
    int64_t range                   // clear Mark if flag+reset+range overflows
)
{

    GB_thread_local.Mark_flag += reset ;

    if (GB_thread_local.Mark_flag + range <= 0 || reset == 0)
    {
        // integer overflow has occurred; clear all of the Mark array
        for (int64_t i = 0 ; i < GB_thread_local.Mark_size ; i++)
        {
            GB_thread_local.Mark [i] = 0 ;
        }
        GB_thread_local.Mark_flag = 1 ;
    }

    // assertion for debugging only:
    ASSERT_MARK_IS_RESET ;          // assert that Mark [...] < flag

    return (GB_thread_local.Mark_flag) ;
}

