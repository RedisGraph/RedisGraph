//------------------------------------------------------------------------------
// GB_Pending.h: data structure and operations for pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_PENDING_H
#define GB_PENDING_H
#include "GB.h"

//------------------------------------------------------------------------------
// GB_Pending data structure
//------------------------------------------------------------------------------

struct GB_Pending_struct    // list of pending tuples for a matrix
{
    int64_t n ;         // number of pending tuples to add to matrix
    int64_t nmax ;      // size of i,j,x
    bool sorted ;       // true if pending tuples are in sorted order
    int64_t *i ;        // row indices of pending tuples
    int64_t *j ;        // col indices of pending tuples; NULL if A->vdim <= 1
    GB_void *x ;        // values of pending tuples
    GrB_Type type ;     // the type of s
    size_t size ;       // type->size
    GrB_BinaryOp op ;   // operator to assemble pending tuples
} ;

// initial size of the pending tuples
#define GB_PENDING_INIT 256

//------------------------------------------------------------------------------
// GB_Pending functions
//------------------------------------------------------------------------------

bool GB_Pending_alloc       // create a list of pending tuples
(
    GB_Pending *PHandle,    // output
    GrB_Type type,          // type of pending tuples
    GrB_BinaryOp op,        // operator for assembling pending tuples
    bool is_matrix,         // true if Pending->j must be allocated
    int64_t nmax            // # of pending tuples to hold
) ;

bool GB_Pending_realloc     // reallocate a list of pending tuples
(
    GB_Pending *PHandle,    // Pending tuple list to reallocate
    int64_t nnew            // # of new tuples to accomodate
) ;

void GB_Pending_free        // free a list of pending tuples
(
    GB_Pending *PHandle
) ;

bool GB_Pending_ensure      // create or reallocate a list of pending tuples
(
    GB_Pending *PHandle,    // input/output
    GrB_Type type,          // type of pending tuples
    GrB_BinaryOp op,        // operator for assembling pending tuples
    bool is_matrix,         // true if Pending->j must be allocated
    int64_t nnew            // # of pending tuples to add
) ;

//------------------------------------------------------------------------------
// GB_Pending_add:  add an entry A(i,j) to the list of pending tuples
//------------------------------------------------------------------------------

static inline bool GB_Pending_add   // add a tuple to the list
(
    GB_Pending *PHandle,        // Pending tuples to create or append
    const GB_void *scalar,      // scalar to add to the pending tuples
    const GrB_Type type,        // scalar type, if list is created
    const GrB_BinaryOp op,      // new Pending->op, if list is created
    const int64_t i,            // index into vector
    const int64_t j,            // vector index
    const bool is_matrix        // allocate Pending->j, if list is created
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (PHandle != NULL) ;
    GB_Pending Pending = (*PHandle) ;

    //--------------------------------------------------------------------------
    // allocate the Pending tuples, or ensure existing list is large enough
    //--------------------------------------------------------------------------

    int64_t n = 0 ;

    if (Pending == NULL)
    { 
        // this is the first pending tuple: define the type of the pending
        // tuples, and the operator to eventually be used to assemble them.
        // If op is NULL, the implicit SECOND_Atype operator will be used.
        if (!GB_Pending_alloc (PHandle, type, op, is_matrix, GB_PENDING_INIT))
        {
            return (false) ;
        }
        Pending = (*PHandle) ;
    }
    else
    {
        n = Pending->n ;
        if (n == Pending->nmax)
        { 
            // reallocate the list so it can hold the new tuple
            if (!GB_Pending_realloc (PHandle, 1))
            {
                return (false) ;
            }
        }
    }

    ASSERT (Pending->type == type) ;
    ASSERT (Pending->nmax > 0 && n < Pending->nmax) ;
    ASSERT (Pending->i != NULL && Pending->x != NULL) ;
    ASSERT ((is_matrix) == (Pending->j != NULL)) ;

    //--------------------------------------------------------------------------
    // keep track of whether or not the pending tuples are already sorted
    //--------------------------------------------------------------------------

    int64_t *restrict Pending_i = Pending->i ;
    int64_t *restrict Pending_j = Pending->j ;

    if (n > 0 && Pending->sorted)
    { 
        int64_t ilast = Pending_i [n-1] ;
        int64_t jlast = (Pending_j != NULL) ? Pending_j [n-1] : 0 ;
        Pending->sorted = (jlast < j) || (jlast == j && ilast <= i) ;
    }

    //--------------------------------------------------------------------------
    // add the (i,j,scalar) or just (i,scalar) if Pending->j is NULL
    //--------------------------------------------------------------------------

    Pending_i [n] = i ;
    if (Pending_j != NULL)
    { 
        Pending_j [n] = j ;
    }
    size_t size = type->size ;
    GB_void *restrict Pending_x = Pending->x ;
    memcpy (Pending_x +(n*size), scalar, size) ;
    Pending->n++ ;

    return (true) ;     // success
}

//------------------------------------------------------------------------------
// add (iC,jC,aij) or just (iC,aij) if Pending_j is NULL
//------------------------------------------------------------------------------

// GB_PENDING_INSERT(aij) is used by GB_subassign to insert a pending tuple, in
// phase 2.  The list has already been reallocated after phase 1 to hold all
// the new pending tuples, so GB_Pending_realloc is not required.

#define GB_PENDING_INSERT(aij)                                              \
    if (task_sorted)                                                        \
    {                                                                       \
        if (!((jlast < jC) || (jlast == jC && ilast <= iC)))                \
        {                                                                   \
            task_sorted = false ;                                           \
        }                                                                   \
    }                                                                       \
    Pending_i [n] = iC ;                                                    \
    if (Pending_j != NULL) Pending_j [n] = jC ;                             \
    memcpy (Pending_x +(n*asize), (aij), asize) ;                           \
    n++ ;                                                                   \
    ilast = iC ;                                                            \
    jlast = jC ;

#endif

