//------------------------------------------------------------------------------
// GB_eslice: uniform partition of e items to each task 
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// There are e items to split between ntasks.  Task tid will own items
// Slice [tid] to Slice [tid+1]-1.

#include "GB.h"

void GB_eslice
(
    int64_t *Slice,         // array of size ntasks+1
    int64_t e,              // number items to partition amongst the tasks
    const int ntasks        // # of tasks
)
{
    Slice [0] = 0 ;
    for (int tid = 0 ; tid < ntasks ; tid++)
    { 
        Slice [tid] = GB_PART (tid, e, ntasks) ;
    }
    Slice [ntasks] = e ;
}

