//------------------------------------------------------------------------------
// GB_pending_add:  add an entry A(i,j) to the list of pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Compare this function with the CSparse function cs_entry, the essence of
// which is copied below.  A CSparse matrix can be held in either compressed
// sparse column format, or as a list of tuples, but never both.  A GraphBLAS
// matrix can have both components.

// The cs_entry function appends a single entry to the end of the tuple list,
// and it doubles the space if no space is available.  It also augments the
// matrix dimension as needed, which GB_pending_add does not do.

// cs_entry (c)2006-2016, T. A. Davis, included here with the GraphBLAS license

//  /* add an entry to a triplet matrix; return 1 if ok, 0 otherwise */
//  int cs_entry (cs *T, int64_t i, int64_t j, double scalar)
//  {
//      if (!CS_TRIPLET (T) || i < 0 || j < 0) return (0) ;
//      if (T->nz >= T->nzmax && !cs_sprealloc (T,2*(T->nzmax))) return (0) ;
//      if (T->x) T->x [T->nz] = scalar ;
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

// If the function succeeds, the matrix is added to the queue if it is not
// already there.

// If the function fails to add the pending tuple, the entire matrix is
// cleared of all entries, all pending tuples, and all zombies; and it is
// removed from the queue if it is already there.

// This function is agnostic about the CSR/CSC format of A.  Regardless of the
// format, i refers to an index into the vectors, and j is a vector.  So for
// CSC, i is a row index and j is a column index.  For CSR, i is a column index
// and j is a row index.  This function also does not need to know if A is
// hypersparse or not.

#include "GB.h"

GrB_Info GB_pending_add             // add a pending tuple A(i,j) to a matrix
(
    GrB_Matrix A,                   // matrix to modify
    const void *scalar,             // scalar to add to the pending tuples of A
    const GrB_Type stype,           // scalar type
    const GrB_BinaryOp pop,         // new A->operator_pending, if 1st pending
    const int64_t i,                // index into vector
    const int64_t j,                // vector index
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // pending tuples are OK; that is the whole point of this function
    ASSERT (GB_PENDING_OK (A)) ;

    // likesize, the matrix A may have zombies
    ASSERT (GB_ZOMBIES_OK (A)) ;

    ASSERT (A->n_pending <= A->max_n_pending) ;
    ASSERT (GB_IMPLIES (A->max_n_pending == 0,
        A->i_pending == NULL && A->j_pending == NULL && A->s_pending == NULL)) ;
    ASSERT (i >= 0 && i < A->vlen && j >= 0 && j < A->vdim) ;

    //--------------------------------------------------------------------------
    // get the type of the pending tuples of A
    //--------------------------------------------------------------------------

    if (A->n_pending == 0)
    { 
        // this is the first pending tuple: define the type of the pending
        // tuples, and the operator to eventually be used to assemble them.
        // If pop is NULL, the implicit SECOND_Atype operator will be used.
        A->type_pending = stype ;
        A->type_pending_size = stype->size ;
        A->operator_pending = pop ;
    }

    // A->s_pending is an array of GrB_Type stype
    ASSERT (A->type_pending == stype) ;
    size_t ssize = stype->size ;

    //--------------------------------------------------------------------------
    // increase the space if not big enough
    //--------------------------------------------------------------------------

    // If this is the first tuple, then A->n_pending == A->max_n_pending == 0.

    if (A->n_pending == A->max_n_pending)
    {
        int64_t newsize = GB_IMAX (2 * A->max_n_pending, 256) ;
        bool ok1 = true ;
        bool ok2 = true ;
        bool ok3 = true ;

        // if A->max_n_pending is zero, these calls to GB_REALLOC_MEMORY
        // allocate new space.

        double memory = GBYTES (newsize, sizeof (int64_t)) ;
        GB_REALLOC_MEMORY (A->i_pending, newsize, A->max_n_pending,
            sizeof (int64_t), &ok1) ;

        if (A->vdim > 1)
        { 
            // do not allocate A->j_pending if A has just one column
            memory += GBYTES (newsize, sizeof (int64_t)) ;
            GB_REALLOC_MEMORY (A->j_pending, newsize, A->max_n_pending,
                sizeof (int64_t), &ok2) ;
        }

        memory += GBYTES (newsize, ssize) ;
        GB_REALLOC_MEMORY (A->s_pending, newsize, A->max_n_pending,
            ssize, &ok3) ;

        if (!ok1 || !ok2 || !ok3)
        { 
            // out of memory
            GB_CONTENT_FREE (A) ;
            return (GB_OUT_OF_MEMORY (memory)) ;
        }

        A->max_n_pending = newsize ;
    }

    ASSERT (A->max_n_pending > 0 && A->n_pending < A->max_n_pending) ;
    ASSERT (A->i_pending != NULL && A->s_pending != NULL) ;
    ASSERT ((A->vdim > 1) == (A->j_pending != NULL)) ;

    //--------------------------------------------------------------------------
    // keep track of whether or not the pending tuples are already sorted
    //--------------------------------------------------------------------------

    // A->sorted_pending starts out true when there are no pending tuples.
    // When the 2nd and subsequent tuples are added, and A->sorted_pending is
    // still true, then it may become false.  Once it is false and remaining
    // tests are skipped.
    if (A->n_pending > 0 && A->sorted_pending)
    { 
        int64_t ilast = A->i_pending [A->n_pending-1] ;
        int64_t jlast = (A->vdim > 1) ? (A->j_pending [A->n_pending-1]) : 0 ;
        A->sorted_pending = (jlast < j) || (jlast == j && ilast <= i) ;
    }

    //--------------------------------------------------------------------------
    // add the (i,j,scalar) or just (i,scalar) if A has a single column
    //--------------------------------------------------------------------------

    A->i_pending [A->n_pending] = i ;

    // If A->vdim <= 1 the column index j is not needed.
    if (A->vdim > 1)
    { 
        A->j_pending [A->n_pending] = j ;
    }

    // s_pending [n_pending] = scalar
    GB_void *As = A->s_pending ;
    memcpy (As +(A->n_pending*ssize), scalar, ssize) ;

    A->n_pending++ ;

    //--------------------------------------------------------------------------
    // insert A in the queue if it isn't already queued
    //--------------------------------------------------------------------------

    ASSERT (GB_PENDING (A)) ;
    if (!(A->enqueued))
    { 
        GB_CRITICAL (GB_queue_insert (A)) ;
    }
    return (GrB_SUCCESS) ;
}

