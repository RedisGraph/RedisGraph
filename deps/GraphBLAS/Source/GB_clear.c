//------------------------------------------------------------------------------
// GB_clear: clears the content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// All content of A is freed (or removed if shallow) and new A->p and A->h
// content is created.  This puts the matrix A in the same initialized state it
// had after GrB_Matrix_new (&A, ...), with A->magic == GB_MAGIC to denote a
// valid, initialized matrix, with nnz(A) equal to zero.  The dimensions, type,
// and CSR/CSC format are unchanged.  The hypersparsity of the newly empty
// matrix A is determined by the A->hyper_switch for the matrix.  The matrix is
// valid.

// However, if this method runs out of memory, and the A->p and A->h structure
// cannot be recreated, then all content of the matrix is freed or removed, and
// the matrix A is left in an invalid state (A->magic == GB_MAGIC2).  Only the
// header is left.

// A is first converted to sparse or hypersparse, and then conformed via
// GB_conform.  If A->sparsity disables the sparse and hypersparse structures,
// A is converted bitmap instead.

#include "GB.h"

GrB_Info GB_clear           // clear a matrix, type and dimensions unchanged
(
    GrB_Matrix A,           // matrix to clear
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;
    ASSERT (A->magic == GB_MAGIC || A->magic == GB_MAGIC2) ;

    // zombies and pending tuples have no effect; about to delete them anyway
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_PENDING_OK (A)) ;

    //--------------------------------------------------------------------------
    // clear the content of A if bitmap
    //--------------------------------------------------------------------------

    int sparsity = GB_sparsity_control (A->sparsity, A->vdim) ;
    if (((sparsity & (GxB_SPARSE + GxB_HYPERSPARSE)) == 0) && GB_IS_BITMAP (A))
    { 
        // A should remain bitmap
        GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
        GB_memset (A->b, 0, GB_NNZ_HELD (A), nthreads_max) ;
        A->nvals = 0 ;
        A->magic = GB_MAGIC ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // clear the content of A
    //--------------------------------------------------------------------------

    // free all content
    GB_phbix_free (A) ;

    // no more zombies or pending tuples
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    //--------------------------------------------------------------------------
    // allocate new A->p and A->h components
    //--------------------------------------------------------------------------

    // By default, an empty matrix with n > 1 vectors is held in hypersparse
    // form.  A GrB_Matrix with n <= 1, or a GrB_Vector (with n == 1) is always
    // non-hypersparse.  If A->hyper_switch is negative, A will be always be
    // non-hypersparse.

    if (GB_convert_hyper_to_sparse_test (A->hyper_switch, 0, A->vdim))
    {

        //----------------------------------------------------------------------
        // A is sparse
        //----------------------------------------------------------------------

        int64_t plen = A->vdim ;
        A->nvec = plen ;
        A->plen = plen ;
        A->p = GB_CALLOC (plen+1, int64_t) ;
        ASSERT (A->h == NULL) ;
        if (A->p == NULL)
        { 
            // out of memory
            GB_phbix_free (A) ;
            return (GrB_OUT_OF_MEMORY) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // A is hypersparse
        //----------------------------------------------------------------------

        int64_t plen = GB_IMIN (1, A->vdim) ;
        A->nvec = 0 ;
        A->plen = plen ;
        A->p = GB_CALLOC (plen+1, int64_t) ;
        A->h = GB_CALLOC (plen  , int64_t) ;
        if (A->p == NULL || A->h == NULL)
        { 
            // out of memory
            GB_phbix_free (A) ;
            return (GrB_OUT_OF_MEMORY) ;
        }
    }

    A->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // conform A to its desired sparsity 
    //--------------------------------------------------------------------------

    return (GB_conform (A, Context)) ;
}

