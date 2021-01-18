//------------------------------------------------------------------------------
// GB_Pending.h: operations for pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_PENDING_H
#define GB_PENDING_H
#include "GB.h"

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

//------------------------------------------------------------------------------
// GB_Pending_ensure: make sure the list of pending tuples is large enough
//------------------------------------------------------------------------------

// create or reallocate a list of pending tuples

static inline bool GB_Pending_ensure
(
    GB_Pending *PHandle,    // input/output
    GrB_Type type,          // type of pending tuples
    GrB_BinaryOp op,        // operator for assembling pending tuples
    bool is_matrix,         // true if Pending->j must be allocated
    int64_t nnew            // # of pending tuples to add
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (PHandle != NULL) ;

    //--------------------------------------------------------------------------
    // ensure the list of pending tuples is large enough
    //--------------------------------------------------------------------------

    if ((*PHandle) == NULL)
    {
        return (GB_Pending_alloc (PHandle, type, op, is_matrix, nnew)) ;
    }
    else
    {
        return (GB_Pending_realloc (PHandle, nnew)) ;
    }
}

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

    //--------------------------------------------------------------------------
    // allocate the Pending tuples, or ensure existing list is large enough
    //--------------------------------------------------------------------------

    if (!GB_Pending_ensure (PHandle, type, op, is_matrix, 1))
    {
        return (false) ;
    }
    GB_Pending Pending = (*PHandle) ;
    int64_t n = Pending->n ;

    ASSERT (Pending->type == type) ;
    ASSERT (Pending->nmax > 0 && n < Pending->nmax) ;
    ASSERT (Pending->i != NULL && Pending->x != NULL) ;
    ASSERT ((is_matrix) == (Pending->j != NULL)) ;

    //--------------------------------------------------------------------------
    // keep track of whether or not the pending tuples are already sorted
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Pending_i = Pending->i ;
    int64_t *GB_RESTRICT Pending_j = Pending->j ;

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
    GB_void *GB_RESTRICT Pending_x = Pending->x ;
    memcpy (Pending_x +(n*size), scalar, size) ;
    Pending->n++ ;

    return (true) ;     // success
}

//------------------------------------------------------------------------------
// add (iC,jC,aij) or just (iC,aij) if Pending_j is NULL
//------------------------------------------------------------------------------

// GB_PENDING_INSERT(aij) is used by GB_subassign_* to insert a pending tuple,
// in phase 2.  The list has already been reallocated after phase 1 to hold all
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

//------------------------------------------------------------------------------
// GB_shall_block: see if the matrix should be finished
//------------------------------------------------------------------------------

// returns true if GB_Matrix_wait (A) should be done

static inline bool GB_shall_block
(
    GrB_Matrix A
)
{ 

    if (!GB_ANY_PENDING_WORK (A))
    { 
        // no pending work, so no need to block
        return (false) ;
    }
    double npending = (double) GB_Pending_n (A) ;
    double anzmax = ((double) A->vlen) * ((double) A->vdim) ;
    bool many_pending = (npending >= anzmax) ;
    bool blocking = (GB_Global_mode_get ( ) == GrB_BLOCKING) ;
    return (many_pending || blocking) ;
}

#endif

