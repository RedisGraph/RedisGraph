//------------------------------------------------------------------------------
// GB_ix_resize:  reallocate a matrix with some slack for future growth
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// nnz(A) has, or will, change.  The # of nonzeros may decrease significantly,
// in which case the extra space is trimmed.  If the existing space is not
// sufficient, the matrix is doubled in size to accomodate the new entries.

#include "GB.h"

GrB_Info GB_ix_resize           // resize a matrix
(
    GrB_Matrix A,
    const int64_t anz_new,      // required new nnz(A)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // This function is only called by GB_Matrix_wait.  Full and bitmap
    // matrices never have any pending work, so this method is needed only for
    // sparse and hypersparse matrices.
    ASSERT_MATRIX_OK (A, "A to resize", GB0) ;
    ASSERT (!GB_IS_FULL (A)) ;
    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A)) ;

    // This function tolerates pending tuples, zombies, and jumbled matrices.
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_PENDING_OK (A)) ;

    GrB_Info info ;
    int64_t anzmax_orig = A->nzmax ;
    ASSERT (GB_NNZ (A) <= anzmax_orig) ;

    //--------------------------------------------------------------------------
    // resize the matrix
    //--------------------------------------------------------------------------

    ASSERT (anz_new > anzmax_orig) ;
    {

        //----------------------------------------------------------------------
        // grow the space
        //----------------------------------------------------------------------

        // original A->nzmax is not enough; double the matrix space for nnz(A)

        int64_t anzmax_new = 2 * anz_new ;

        // the space is growing so this might run out of memory
        ASSERT (anzmax_new > anzmax_orig) ;
        ASSERT (anz_new <= anzmax_new) ;

        info = GB_ix_realloc (A, anzmax_new, true, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_phbix_free (A) ;
            return (info) ;
        }
        ASSERT_MATRIX_OK (A, "A increased in size", GB0) ;

    }

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    ASSERT (anz_new <= A->nzmax) ;
    return (GrB_SUCCESS) ;
}

