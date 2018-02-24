//------------------------------------------------------------------------------
// GB_add_pending:  add an entry A(i,j) to the list of pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Compare this function with the CSparse function cs_entry, the essence of
// which is copied below.  A CSparse matrix can be held in either compressed
// sparse column format, or as a list of tuples, but never both.  A GraphBLAS
// matrix can have both components.

// The cs_entry function appends a single entry to the end of the tuple list,
// and it doubles the space if no space is available.  It also augments the
// matrix dimension as needed, which GB_add_pending does not do.

// cs_entry (c)2006-2016, T. A. Davis, included here with the GraphBLAS license

//  /* add an entry to a triplet matrix; return 1 if ok, 0 otherwise */
//  int cs_entry (cs *T, int64_t i, int64_t j, double x)
//  {
//      if (!CS_TRIPLET (T) || i < 0 || j < 0) return (0) ;
//      if (T->nz >= T->nzmax && !cs_sprealloc (T,2*(T->nzmax))) return (0) ;
//      if (T->x) T->x [T->nz] = x ;
//      T->i [T->nz] = i ;
//      T->p [T->nz++] = j ;
//      T->m = CS_MAX (T->m, i+1) ;
//      T->n = CS_MAX (T->n, j+1) ;
//      return (1) ;
//  }

// This function starts with an initial list that is larger than cs_entry
// (which starts with a list of size 1), and it quadruples the size as needed
// instead of doubling it.  If A has a single column then the column index is
// not kept.  Finally, this function supports any data type whereas CSparse
// only allows for double.

// Otherwise the two methods are essentially the same.  The reader is
// encouraged the compare/contrast the unique coding styles used in CSparse and
// this implementation of GraphBLAS.  CSparse is concise; the book provides the
// code commentary: Direct Methods for Sparse Linear Systems, Timothy A. Davis,
// SIAM, Philadelphia, Sept. 2006, http://bookstore.siam.org/fa02 .  Csparse is
// at http://faculty.cse.tamu.edu/davis/publications_files/CSparse.zip .

// This function does not keep track of the A->operator_pending, since it is
// not given the current operator.  The caller must ensure that all pending
// tuples share the same operator.

// If the function succeeds, the matrix is added to the queue if it is not
// already there.

// If the function fails to add the pending tuple, the entire matrix is
// cleared of all entries, all pending tuples, and all zombies; and it is
// removed from the queue if it is already there.

#include "GB.h" 

#define INITIAL_NPENDING_MAX 256

GrB_Info GB_add_pending         // add a pending tuple A(i,j) to a matrix
(
    GrB_Matrix A,               // matrix to modify
    const void *x,              // scalar to set
    const GB_Type_code xcode,   // type of the scalar x
    const int64_t i,            // row index
    const int64_t j             // column index
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // pending tuples are OK; that is the whole point of this function
    ASSERT (PENDING_OK (A)) ;

    // likesize, the matrix A may have zombies
    ASSERT (ZOMBIES_OK (A)) ;

    size_t asize = A->type->size ;
    ASSERT (A->npending <= A->max_npending) ;
    ASSERT (IMPLIES (A->max_npending == 0,
         A->ipending == NULL && A->jpending == NULL && A->xpending == NULL)) ;
    ASSERT (i < A->nrows && j < A->ncols) ;

    //--------------------------------------------------------------------------
    // increase the space if not big enough
    //--------------------------------------------------------------------------

    // If this is the first tuple, then A->npending == A->max_npending == 0.

    if (A->npending == A->max_npending)
    {
        int64_t newsize = IMAX (2 * A->max_npending, INITIAL_NPENDING_MAX) ;
        bool ok1 = true ;
        bool ok2 = true ;
        bool ok3 = true ;

        // if A->max_npending is zero, these calls to GB_REALLOC_MEMORY
        // allocate new space.

        GB_REALLOC_MEMORY (A->ipending, newsize, A->max_npending,
            sizeof (int64_t), &ok1) ;

        if (A->ncols > 1)
        {
            // do not allocate A->jpending if A has just one column
            GB_REALLOC_MEMORY (A->jpending, newsize, A->max_npending,
                sizeof (int64_t), &ok2) ;
        }

        GB_REALLOC_MEMORY (A->xpending, newsize, A->max_npending, asize, &ok3) ;

        if (!ok1 || !ok2 || !ok3)
        {
            // out of memory; clear all of A and remove from the queue
            GB_Matrix_clear (A) ;
            ASSERT (!(A->enqueued)) ;
            ASSERT (!PENDING (A)) ;
            ASSERT (!ZOMBIES (A)) ;
            ASSERT_OK (GB_check (A, "A failed to add", 0)) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "Out of memory for list of pending tuples"))) ;
        }

        A->max_npending = newsize ;
    }

    ASSERT (A->max_npending > 0 && A->npending < A->max_npending) ;
    ASSERT (A->ipending != NULL && A->xpending != NULL) ;
    ASSERT ((A->ncols > 1) == (A->jpending != NULL)) ;

    //--------------------------------------------------------------------------
    // keep track of whether or not the pending tuples are already sorted
    //--------------------------------------------------------------------------

    // A->sorted_pending starts out true when there are no pending tuples.
    // When the 2nd and subsequent tuples are added, and A->sorted_pending is
    // still true, then it may become false.  Once it is false and remaining
    // tests are skipped.
    if (A->npending > 0 && A->sorted_pending)
    {
        int64_t ilast = A->ipending [A->npending-1] ;
        int64_t jlast = (A->ncols > 1) ? (A->jpending [A->npending-1]) : 0 ;
        A->sorted_pending = (jlast < j) || (jlast == j && ilast <= i) ;
    }

    //--------------------------------------------------------------------------
    // add the (i,j,x) tuple to the list, or just (i,x) if A has a single column
    //--------------------------------------------------------------------------

    A->ipending [A->npending] = i ;

    // If A->ncols <= 1 the column index j is not needed.
    if (A->ncols > 1)
    {
        A->jpending [A->npending] = j ;
    }

    ASSERT (xcode <= GB_UDT_code) ;
    if (xcode == GB_UDT_code || xcode == A->type->code)
    {
        // copy the values without typecasting
        memcpy (A->xpending +(A->npending*asize), x, asize) ;
    }
    else
    {
        // typecast the value from x into A
        GB_cast_array (A->xpending +(A->npending*asize), A->type->code,
            x, xcode, 1) ;
    }

    A->npending++ ;

    //--------------------------------------------------------------------------
    // insert A in the queue if it isn't already queued
    //--------------------------------------------------------------------------

    ASSERT (PENDING (A)) ;
    if (!(A->enqueued))
    {
        GB_queue_insert (A) ;
    }
    return (GrB_SUCCESS) ;
}

#undef INITIAL_NPENDING_MAX

