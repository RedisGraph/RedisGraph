//------------------------------------------------------------------------------
// GB_Pending_alloc: allocate a list of pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_Pending.h"

bool GB_Pending_alloc       // create a list of pending tuples
(
    GB_Pending *PHandle,    // output
    GrB_Type type,          // type of pending tuples
    GrB_BinaryOp op,        // operator for assembling pending tuples
    bool is_matrix,         // true if Pending->j must be allocated
    int64_t nmax            // # of pending tuples to hold
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (PHandle != NULL) ;
    (*PHandle) = NULL ;

    //--------------------------------------------------------------------------
    // allocate the Pending header
    //--------------------------------------------------------------------------

    GB_Pending Pending = GB_CALLOC (1, struct GB_Pending_struct) ;
    if (Pending == NULL)
    { 
        // out of memory
        return (false) ;
    }

    //--------------------------------------------------------------------------
    // initialize the contents of the Pending tuples
    //--------------------------------------------------------------------------

    nmax = GB_IMAX (nmax, GB_PENDING_INIT) ;
    Pending->n = 0 ;                    // no pending tuples yet
    Pending->nmax = nmax ;              // initial size of list
    Pending->sorted = true ;            // keep track if tuples are sorted
    Pending->type = type ;              // type of pending tuples
    Pending->size = type->size ;        // size of pending tuple type
    Pending->op = op ;                  // pending operator (NULL is OK)

    Pending->i = GB_MALLOC (nmax, int64_t) ;

    if (is_matrix)
    { 
        Pending->j = GB_MALLOC (nmax, int64_t) ;
    }
    else
    { 
        Pending->j = NULL ;
    }

    Pending->x = GB_MALLOC (nmax * Pending->size, GB_void) ;

    if (Pending->i == NULL || Pending->x == NULL
        || (is_matrix && Pending->j == NULL))
    { 
        // out of memory
        GB_Pending_free (&Pending) ;
        return (false) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*PHandle) = Pending ;
    return (true) ;
}

