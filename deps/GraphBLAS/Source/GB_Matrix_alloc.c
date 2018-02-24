//------------------------------------------------------------------------------
// GB_Matrix_alloc: allocate a matrix to hold a given number of entries
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Does not modify A->p.  Frees A->x and A->i and reallocates them to the
// requested size.  Frees any pending tuples and deletes all entries (including
// zombies, if any).  If numeric is false, then A->x is freed but not
// reallocated.

#include "GB.h"

bool GB_Matrix_alloc        // allocate space in a matrix
(
    GrB_Matrix A,           // matrix to allocate space for
    const GrB_Index nzmax,  // number of entries the matrix can hold
    const bool numeric,     // if true, allocate A->x, otherwise A->x is NULL
    double *memory          // memory required
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // GB_new does not always initialize A->p; GB_check fails in this case.  So
    // the following assertion is not possible here.  This is by design.
    // Thus, ASSERT_OK (GB_check (A, "A", 0)) ;  cannot be used here.
    ASSERT (A != NULL && A->p != NULL) ;

    if (memory != NULL)
    {
        *memory += GBYTES (nzmax,
            sizeof (int64_t) + (numeric ? A->type->size : 0)) ;
    }

    if (nzmax > GB_INDEX_MAX)
    {
        // problem too large.  The caller treats this the same as an
        // out-of-memory condition.  This is reasonable since memory of size
        // nzmax * (sizeof (int64_t) + sizeof (type)) is about to be allocated.
        // GB_INDEX_MAX is 2^60, and sizeof(int64_t) is 2^3, so the 2^63 bytes
        // of memory would be required.
        return (false) ;
    }

    //--------------------------------------------------------------------------
    // allocate the A->x and A->i content of the matrix
    //--------------------------------------------------------------------------

    // free the existing content, if any.  Leave A->p unchanged.
    GB_Matrix_ixfree (A) ;

    // allocate the new A->x and A->i content
    A->nzmax = IMAX (nzmax, 1) ;
    GB_MALLOC_MEMORY (A->i, A->nzmax, sizeof (int64_t)) ;
    if (numeric)
    {
        GB_MALLOC_MEMORY (A->x, A->nzmax, A->type->size) ;
    }
    if (A->i == NULL || (numeric && A->x == NULL))
    {
        // out of memory
        GB_FREE_MEMORY (A->x, A->nzmax, A->type->size) ;
        GB_FREE_MEMORY (A->i, A->nzmax, sizeof (int64_t)) ;
        return (false) ;
    }

    return (true) ;
}

