//------------------------------------------------------------------------------
// GB_heap_build: construct a Heap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// On input, the Heap [1..nheap] may not satisfy the min-heap property.
// On output, the elements have been rearranged so that it does.

#include "GB.h"
#include "GB_heap.h"

void GB_heap_build
(
    GB_Element *restrict Heap,  // Heap [1..nheap]; modified
    const int64_t nheap         // the number of nodes in the Heap
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Heap != NULL && nheap >= 0) ;

    //--------------------------------------------------------------------------
    // build the Heap
    //--------------------------------------------------------------------------

    for (int64_t p = nheap / 2 ; p >= 1 ; p--)
    { 
        GB_heapify (p, Heap, nheap) ;
    }

    //--------------------------------------------------------------------------
    // check result
    //--------------------------------------------------------------------------

    // Heap [1..nheap] now satisfies the min-heap property
    ASSERT (GB_heap_check (Heap, nheap)) ;
}

