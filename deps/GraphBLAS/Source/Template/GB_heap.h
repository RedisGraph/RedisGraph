//------------------------------------------------------------------------------
// GB_heap: a Heap data structure and its operations
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

#ifndef GB_HEAP
#define GB_HEAP

// The Heap is an array of GB_Elements: Heap [1..nheap].  Each entry in the
// Heap is a GB_Element, with a key and name.

#ifndef NDEBUG

//------------------------------------------------------------------------------
// Functions only for assertions
//------------------------------------------------------------------------------

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

    for (int64_t t = p ; t >= 1 ; t = t / 2)
    {
        if (Heap [p].key != Heap [t].key)
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

static inline void GB_heap_build
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

//------------------------------------------------------------------------------
// GB_heap_delete: delete an element in the middle of a Heap
//------------------------------------------------------------------------------

static inline void GB_heap_delete
(
    int64_t p,                  // node that needs to be deleted
    GB_Element *restrict Heap,  // Heap [1..nheap]
    int64_t *restrict nheap     // the number of nodes in the Heap;
                                // decremented on output
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Heap != NULL && (*nheap) >= 0) ;
    ASSERT (p >= 0 && p <= (*nheap)) ;

    //--------------------------------------------------------------------------
    // delete node p from the Heap
    //--------------------------------------------------------------------------

    // move the last node to node p and decrement the # of nodes in the Heap
    Heap [p] = Heap [(*nheap)--] ;

    // heapify node p (safely does nothing if node p was the one just deleted)
    GB_heapify (p, Heap, (*nheap)) ;
}


//------------------------------------------------------------------------------
// GB_heap_getminlist: get a list of all nodes with minimum key
//------------------------------------------------------------------------------

// Constructs a list of all nodes in the Heap with a key equal to Heap [1].key.
// The Heap is not modified.  The list is returned in topological order: If
// node p appears as p = List [k], and if its left child pleft = 2*p is in the
// list at pleft = List [kleft], then k < kleft.  Likewise for its right child,
// pright = 2*p+1.

static inline int64_t GB_heap_getminlist    // returns Heap [1].key
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

    #ifndef NDEBUG
    for (int64_t t = 0 ; t < (*nlist) ; t++)
    {
        // each node p in the List satisfies the path property
        int64_t p = List [t] ;
        ASSERT (GB_heap_pathcheck (p, Heap, nheap)) ;
    }
    #endif

    return (minkey) ;
}

#endif
