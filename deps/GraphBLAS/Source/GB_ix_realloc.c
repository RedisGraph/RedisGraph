//------------------------------------------------------------------------------
// GB_ix_realloc: reallocate a matrix to hold a given number of entries
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Does not modify A->p.  Reallocates A->x and A->i to the requested size,
// preserving the existing content of A->x and A->i.  Preserves pending tuples
// and zombies, if any.  If numeric is false, then A->x is freed instead.

#include "GB.h"

GrB_Info GB_ix_realloc      // reallocate space in a matrix
(
    GrB_Matrix A,           // matrix to allocate space for
    const GrB_Index nzmax,  // new number of entries the matrix can hold
    const bool numeric,     // if true, reallocate A->x, otherwise A->x is NULL
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // GB_new does not always initialize A->p; GB_check fails in this case.  So
    // the following assertion is not possible here.  This is by design.
    // Thus, ASSERT_OK (GB_check (A, "A", ...)) ;  cannot be used here.
    ASSERT (A != NULL && A->p != NULL) ;
    ASSERT (GB_IMPLIES (A->is_hyper, A->h != NULL)) ;
    ASSERT (!A->i_shallow && !A->x_shallow) ;

    // This function tolerates pending tuples and zombies
    ASSERT (GB_PENDING_OK (A)) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;

    double memory = GBYTES (nzmax,
        sizeof (int64_t) + (numeric ? A->type->size : 0)) ;

    if (nzmax > GB_INDEX_MAX)
    { 
        // problem too large
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    //--------------------------------------------------------------------------
    // reallocate the space
    //--------------------------------------------------------------------------

    size_t nzmax1 = GB_IMAX (nzmax, 1) ;
    bool ok1 = true, ok2 = true ;
    GB_REALLOC_MEMORY (A->i, nzmax1, A->nzmax, sizeof (int64_t), &ok1) ;
    if (numeric)
    { 
        GB_REALLOC_MEMORY (A->x, nzmax1, A->nzmax, A->type->size, &ok2) ;
    }
    else
    { 
        GB_FREE_MEMORY (A->x, A->nzmax, A->type->size) ;
    }
    bool ok = ok1 && ok2 ;

    // always succeeds if the space shrinks
    ASSERT (GB_IMPLIES (nzmax1 <= A->nzmax, ok)) ;

    if (ok)
    { 
        A->nzmax = nzmax1 ;
    }

    // The matrix is always left in a valid state.  If realloc fails it just
    // won't have the requested size (and ok is false in this case).

    if (!ok)
    { 
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    return (GrB_SUCCESS) ;
}

