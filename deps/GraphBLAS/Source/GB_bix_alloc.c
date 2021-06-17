//------------------------------------------------------------------------------
// GB_bix_alloc: allocate a matrix to hold a given number of entries
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Does not modify A->p or A->h (unless an error occurs).  Frees A->b, A->x,
// and A->i and reallocates them to the requested size.  Frees any pending
// tuples and deletes all entries (including zombies, if any).  If numeric is
// false, then A->x is freed but not reallocated.

// If this method fails, all content of A is freed (including A->p and A->h).

#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_bix_alloc       // allocate A->b, A->i, and A->x space in a matrix
(
    GrB_Matrix A,           // matrix to allocate space for
    const GrB_Index nzmax,  // number of entries the matrix can hold
    const bool is_bitmap,   // if true, allocate A->b, otherwise A->b is NULL
    const bool bitmap_calloc,   // if true, calloc A->b, otherwise use malloc
    const bool is_sparse,   // if true, allocate A->i, otherwise A->i is NULL
    const bool numeric,     // if true, allocate A->x, otherwise A->x is NULL
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;
    if (nzmax > GxB_INDEX_MAX)
    { 
        // problem too large
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // allocate the A->b, A->x, and A->i content of the matrix
    //--------------------------------------------------------------------------

    // Free the existing A->b, A->x, and A->i content, if any.
    // Leave A->p and A->h unchanged.
    GB_bix_free (A) ;

    // allocate the new A->x and A->i content
    A->nzmax = GB_IMAX (nzmax, 1) ;

    bool ok = true ;
    if (is_sparse)
    { 
        if (A->nzmax <= 1)
        {
            A->i = GB_CALLOC (1, int64_t) ;
        }
        else
        { 
            A->i = GB_MALLOC (A->nzmax, int64_t) ;
        }
        ok = (A->i != NULL) ;
    }
    else if (is_bitmap)
    { 
        if (bitmap_calloc)
        { 
            // content is fully defined
            A->b = GB_CALLOC (A->nzmax, int8_t) ;
            A->magic = GB_MAGIC ;
        }
        else
        { 
            // bitmap is not defined and will be computed by the caller
            A->b = GB_MALLOC (A->nzmax, int8_t) ;
        }
        ok = (A->b != NULL) ;
    }

    if (numeric)
    { 
        A->x = GB_MALLOC (A->nzmax * A->type->size, GB_void) ;
        ok = ok && (A->x != NULL) ;
    }

    if (!ok)
    { 
        // out of memory
        GB_phbix_free (A) ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    return (GrB_SUCCESS) ;
}

