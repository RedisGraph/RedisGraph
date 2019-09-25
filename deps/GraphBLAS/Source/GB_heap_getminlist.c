//------------------------------------------------------------------------------
// GB_heap_getminlist: get a list of all nodes with minimum key
//------------------------------------------------------------------------------ 
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Constructs a list of all nodes in the Heap with a key equal to Heap [1].key.
// The Heap is not modified.  The list is returned in topological order: If
// node p appears as p = List [k], and if its left child pleft = 2*p is in the
// list at pleft = List [kleft], then k < kleft.  Likewise for its right child,
// pright = 2*p+1.

#include "GB.h"
#include "GB_heap.h"

int64_t GB_heap_getminlist      // returns Heap [1].key
(
    const GB_Element *restrict Heap,    // Heap [1..nheap], not modified
    const int64_t nheap,                // the number of nodes in the Heap
    // output
    int64_t *restrict List,     // List [0..nlist-1] is a list of all nodes p
                                // with Heap [p].key == Heap [1].key.  Node 1
                                // is always in the list.  List has size nheap.
    int64_t *restrict nlist     // the size of the List
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_heap_check (Heap, nheap)) ;
    ASSERT (nheap >= 1) ;

    //--------------------------------------------------------------------------
    // start the list with node 1
    //--------------------------------------------------------------------------

    // nothing in the List
    (*nlist) = 0 ;

    // push node 1 on the stack (in workspace at the bottom of the List)
    int64_t top = nheap ;
    List [--top] = 1 ;

    // get the key of node 1
    int64_t minkey = Heap [1].key ;

    //--------------------------------------------------------------------------
    // fill the List while the stack is not empty
    //--------------------------------------------------------------------------

    while (top < nheap)
    {
        // pop the top of the stack
        int64_t p = List [top++] ;

        // append p to the List
        List [(*nlist)++] = p ;

        // push its right child on the stack, if it has the same key
        int64_t pright = 2*p + 1 ;
        if (pright <= nheap && Heap [pright].key == minkey)
        { 
            List [--top] = pright ;
        }

        // push its left child on the stack, if it has the same key
        int64_t pleft = 2*p ;
        if (pleft <= nheap && Heap [pleft].key == minkey)
        { 
            List [--top] = pleft ;
        }
    }

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    for (int64_t klist = 0 ; klist < (*nlist) ; klist++)
    {
        // each node p in the List satisfies the path property
        int64_t p = List [klist] ;
        ASSERT (GB_heap_pathcheck (p, Heap, nheap)) ;
    }
    #endif

    return (minkey) ;
}

