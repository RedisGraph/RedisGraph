//------------------------------------------------------------------------------
// GB_Flag_free: free the Flag workspace array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

void GB_Flag_free ( )               // free the Flag array
{
    int64_t currsize = GB_thread_local.Flag_size ;
    GB_FREE_MEMORY (GB_thread_local.Flag, currsize, sizeof (int8_t)) ;
    GB_thread_local.Flag_size = 0 ;
}

