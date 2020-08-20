//------------------------------------------------------------------------------
// GB_ek_slice_free: free workspace created by GB_ek_slice
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_ek_slice.h"

void GB_ek_slice_free
(
    int64_t *GB_RESTRICT *pstart_slice_handle, // size ntasks+1
    int64_t *GB_RESTRICT *kfirst_slice_handle, // size ntasks
    int64_t *GB_RESTRICT *klast_slice_handle,  // size ntasks
    int ntasks                              // # of tasks
)
{ 
    GB_FREE_MEMORY ((*pstart_slice_handle), ntasks+1, sizeof (int64_t)) ;
    GB_FREE_MEMORY ((*kfirst_slice_handle), ntasks, sizeof (int64_t)) ;
    GB_FREE_MEMORY ((*klast_slice_handle), ntasks, sizeof (int64_t)) ;
}

