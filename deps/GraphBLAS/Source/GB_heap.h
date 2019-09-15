//------------------------------------------------------------------------------
// GB_heap: a Heap data structure and its operations
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The Heap is an array of GB_Elements: Heap [1..nheap].  Each entry in the
// Heap is a GB_Element, with a key and name.

// These functions are only used by the heap method for C=A*B.
// See Source/Template/GB_AxB_heap_mask.c.

#ifndef GB_HEAP
#define GB_HEAP

#ifdef GB_DEBUG

//------------------------------------------------------------------------------
// GB_heap_check: make sure the min-heap property holds for the whole Heap
//------------------------------------------------------------------------------

// Check the entire Heap to see if it has the min-heap property:  for all nodes
// in the Heap, the key of a node must less than or equal to the keys of its
// children  (duplicate keys may appear).  An empty Heap or a Heap of size 1
// always satisfies the min-heap property, but nheap < 0 is invalid.  This
// function is for assertions only.

static inline bool GB_heap_check
(
    const GB_Element *restrict Heap,    // Heap [1..nheap], not modified
    const int64_t nheap                 // the number of nodes in the Heap
)
{

    if (Heap == NULL || nheap < 0)
    {
        // Heap is invalid
        return (false) ;
    }

    // nodes nheap/2 ... nheap have no children, so no need to check them
    for (int64_t p = 1 ; p <= nheap / 2 ; p++)
    {

        // consider node p.  Its key must be <= the key of both its children.

        int64_t pleft  = 2*p ;          // left child of node p
        int64_t pright = pleft + 1 ;    // right child of node p

        if (pleft <= nheap && Heap [p].key > Heap [pleft].key)
        {
            // left child of p is in the Heap, but p has a bigger key;
            // the min-heap property is not satisfied
            return (false) ;
        }

        if (pright <= nheap && Heap [p].key > Heap [pright].key)
        {
            // left child of p is in the Heap, but p has a bigger key;
            // the min-heap property is not satisfied
            return (false) ;
        }
    }

    // Heap is OK and satisfies the min-heap property
    return (true) ;
}

//------------------------------------------------------------------------------
// GB_heap_path_check: make sure a path in the Heap is valid
//------------------------------------------------------------------------------

// The path from node p up to the root node 1 (node p, parent(p),
// parent(parent(p)) ... to node 1) must all have the same key where
// parent(p)=p/2.

static inline bool GB_heap_pathcheck
(
    int64_t p,                          // node to check, in range 1..nheap
    const GB_Element *restrict Heap,    // Heap [1..nheap], not modified
    const int64_t nheap                 // the number of nodes in the Heap
)
{

    if (Heap == NULL || nheap < 0 || p < 1 || p > nheap)
    {
        // Heap is invalid or node p is not in the range 1..nheap
        return (false) ;
    }

    for (int64_t kheap = p ; kheap >= 1 ; kheap = kheap / 2)
    {
        if (Heap [p].key != Heap [kheap].key)
        {
            // key of node p does not match one if its ancestors
            return (false) ;
        }
    }

    // all nodes from p to the root have the same key
    return (true) ;
}

#endif

//------------------------------------------------------------------------------
// GB_heapify: enforce the min-heap property of a node
//------------------------------------------------------------------------------

// Heapify starting at node p in the Heap.  On input, the Heap rooted at node p
// satisfies the min-heap property, except for Heap [p] itself.  On output, all
// of the Heap rooted at node p satisfies the min-heap property.

static inline void GB_heapify
(
    int64_t p,                      // node that needs to be heapified
    GB_Element *restrict Heap,      // Heap [1..nheap]; modified
    const int64_t nheap             // the number of nodes in the Heap
)
{

    //--------------------------------------------------------------------------
    // check inputs and check for quick return
    //--------------------------------------------------------------------------

    ASSERT (Heap != NULL) ;

    if (p > nheap / 2 || nheap <= 1)
    { 
        // nothing to do.  p has no children in the Heap.
        // Also safely do nothing if p is outside the Heap (p > nheap).
        return ;
    }

    //--------------------------------------------------------------------------
    // get the element to heapify
    //--------------------------------------------------------------------------

    // Get the element e at node p in the Heap; the one that needs heapifying.
    GB_Element e = Heap [p] ;

    // There is now a "hole" at Heap [p], with no element in it.

    //--------------------------------------------------------------------------
    // heapify
    //--------------------------------------------------------------------------

    while (true)
    {

        //----------------------------------------------------------------------
        // consider node p in the Heap
        //----------------------------------------------------------------------

        // Heap [p] is the "hole" in the Heap

        int64_t pleft  = 2*p ;          // left child of node p
        int64_t pright = pleft + 1 ;    // right child of node p

        if (pright <= nheap)
        {

            //------------------------------------------------------------------
            // both left and right children are in the Heap
            //------------------------------------------------------------------

            if (Heap [pleft].key < Heap [pright].key)
            {
                // left node has a smaller key than the right node
                if (e.key > Heap [pleft].key)
                { 
                    // key of element e is bigger than the left child of p, so
                    // bubble up the left child into the hole at Heap [p] and
                    // continue down the left child.  The hole moves to node
                    // pleft.
                    Heap [p] = Heap [pleft] ;
                    p = pleft ;
                }
                else
                { 
                    // done!  key of element e is is smaller than the left
                    // child of p; place e in the hole at p, and we're done.
                    Heap [p] = e ;
                    return ;
                }
            }
            else
            {
                // right node has a smaller key than the left node
                if (e.key > Heap [pright].key)
                { 
                    // key of element e is bigger than the right child of p, so
                    // bubble up the right child into hole at Heap [p] and
                    // continue down the right child.  The hole moves to node
                    // pright.
                    Heap [p] = Heap [pright] ;
                    p = pright ;
                }
                else
                { 
                    // done!  key of element e is is smaller than the right
                    // child of p; place e in the hole at p, and we're done.
                    Heap [p] = e ;
                    return ;
                }
            }
        }
        else
        {

            //------------------------------------------------------------------
            // right child is not in the Heap, see if left child is in the Heap
            //------------------------------------------------------------------

            if (pleft <= nheap)
            {
                // left child is in the Heap; check its key
                if (e.key > Heap [pleft].key)
                { 
                    // key of element e is bigger than the left child of p, so
                    // bubble up the left child into the hole at Heap [p] and
                    // continue down the left child.  The hole moves to node
                    // pleft.
                    Heap [p] = Heap [pleft] ;
                    p = pleft ;
                }
            }

            //------------------------------------------------------------------
            // node p is a hole, and it has no children
            //------------------------------------------------------------------

            // put e in the hole, and we're done
            Heap [p] = e ;
            return ;
        }
    }
}

//------------------------------------------------------------------------------
// GB_heap_build: construct a Heap
//------------------------------------------------------------------------------

// On input, the Heap [1..nheap] may not satisfy the min-heap property.
// On output, the elements have been rearranged so that it does.

void GB_heap_build
(
    GB_Element *restrict Heap,  // Heap [1..nheap]; modified
    const int64_t nheap         // the number of nodes in the Heap
) ;

//------------------------------------------------------------------------------
// GB_heap_delete: delete an element in the middle of a Heap
//------------------------------------------------------------------------------

void GB_heap_delete
(
    int64_t p,                  // node that needs to be deleted
    GB_Element *restrict Heap,  // Heap [1..nheap]
    int64_t *restrict nheap     // the number of nodes in the Heap;
                                // decremented on output
) ;

//------------------------------------------------------------------------------
// GB_heap_getminlist: get a list of all nodes with minimum key
//------------------------------------------------------------------------------

int64_t GB_heap_getminlist      // returns Heap [1].key
(
    const GB_Element *restrict Heap,    // Heap [1..nheap], not modified
    const int64_t nheap,                // the number of nodes in the Heap
    // output
    int64_t *restrict List,     // List [0..nlist-1] is a list of all nodes p
                                // with Heap [p].key == Heap [1].key.  Node 1
                                // is always in the list.  List has size nheap.
    int64_t *restrict nlist     // the size of the List
) ;

#endif

