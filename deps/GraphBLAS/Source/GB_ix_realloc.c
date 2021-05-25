//------------------------------------------------------------------------------
// GB_ix_realloc: reallocate a matrix to hold a given number of entries
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Does not modify A->p.  Reallocates A->x and A->i to the requested size,
// preserving the existing content of A->x and A->i.  Preserves pending tuples
// and zombies, if any.  If numeric is false, then A->x is freed instead.

#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_ix_realloc      // reallocate space in a matrix
(
    GrB_Matrix A,               // matrix to allocate space for
    const int64_t nzmax_new,    // new number of entries the matrix can hold
    const bool numeric,         // if true, reallocate A->x, else A->x is NULL
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // This method is used only by GB_ix_resize, which itself is used only by
    // GrB_Matrix_wait.  Full and bitmap matrices never have pending work, so
    // this function is only called for hypersparse and sparse matrices.
    ASSERT (!GB_IS_FULL (A)) ;
    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A)) ;

    // A->p has been allocated but might not be initialized.  GB_Matrix_check
    // fails in this case.  Thus, ASSERT_MATRIX_OK (A, "A", ...) ;  cannot be
    // used here.
    ASSERT (A != NULL && A->p != NULL) ;
    ASSERT (!A->i_shallow && !A->x_shallow) ;

    // This function tolerates pending tuples, zombies, and jumbled matrices.
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_PENDING_OK (A)) ;

    if (nzmax_new > GxB_INDEX_MAX)
    { 
        // problem too large
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // reallocate the space
    //--------------------------------------------------------------------------

    size_t nzmax_new1 = GB_IMAX (nzmax_new, 1) ;
    bool ok1 = true, ok2 = true ;
    GB_REALLOC (A->i, nzmax_new1, A->nzmax, int64_t, &ok1) ;
    if (numeric)
    { 
        size_t asize = A->type->size ;
        GB_REALLOC (A->x, nzmax_new1*asize, (A->nzmax)*asize, GB_void, &ok2) ;
    }
    else
    { 
        GB_FREE (A->x) ;
    }
    bool ok = ok1 && ok2 ;

    // always succeeds if the space shrinks
    ASSERT (GB_IMPLIES (nzmax_new1 <= A->nzmax, ok)) ;

    if (ok)
    { 
        A->nzmax = nzmax_new1 ;
    }

    // The matrix is always left in a valid state.  If the reallocation fails
    // it just won't have the requested size (and ok is false in this case).
    if (!ok)
    { 
        return (GrB_OUT_OF_MEMORY) ;
    }

    return (GrB_SUCCESS) ;
}

