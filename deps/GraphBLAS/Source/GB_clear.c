//------------------------------------------------------------------------------
// GB_clear: clears the content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// All content of A is freed (or removed if shallow) and new A->p and A->h
// content is created.  This puts the matrix A in the same initialized state it
// had after GrB_Matrix_new (&A, ...), with A->magic == GB_MAGIC to denote a
// valid, initialized matrix, with nnz(A) equal to zero.  The dimensions, type,
// and CSR/CSC format are unchanged.  The hypersparsity of the newly empty
// matrix A is determined by the A->hyper_ratio for the matrix.  The matrix is
// valid.

// However, if this method runs out of memory, and the A->p and A->h structure
// cannot be recreated, then all content of the matrix is freed or removed, and
// the matrix A is left in an invalid state (A->magic == GB_MAGIC2).  Only the
// header is left.

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
    ASSERT (GB_PENDING_OK (A)) ; ASSERT (GB_ZOMBIES_OK (A)) ;

    //--------------------------------------------------------------------------
    // clear the content of A
    //--------------------------------------------------------------------------

    // free all content
    GB_PHIX_FREE (A) ;

    // no more zombies or pending tuples
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    //--------------------------------------------------------------------------
    // check hypersparsity status of an empty matrix
    //--------------------------------------------------------------------------

    // By default, an empty matrix with n > 1 vectors is held in hypersparse
    // form.  A GrB_Matrix with n <= 1, or a GrB_Vector (with n == 1) is always
    // non-hypersparse.  If A->hyper_ratio is negative, A will be always be
    // non-hypersparse.

    A->is_hyper = true ;
    A->nvec_nonempty = 0 ;
    if (GB_to_nonhyper_test (A, A->nvec_nonempty, A->vdim))
    { 
        A->is_hyper = false ;
    }

    //--------------------------------------------------------------------------
    // allocate new A->p and A->h components
    //--------------------------------------------------------------------------

    if (A->is_hyper)
    {

        //----------------------------------------------------------------------
        // A is hypersparse
        //----------------------------------------------------------------------

        int64_t plen = GB_IMIN (1, A->vdim) ;
        A->nvec = 0 ;
        A->plen = plen ;
        GB_CALLOC_MEMORY (A->p, plen+1, sizeof (int64_t)) ;
        GB_CALLOC_MEMORY (A->h, plen  , sizeof (int64_t)) ;
        if (A->p == NULL || A->h == NULL)
        { 
            // out of memory
            GB_PHIX_FREE (A) ;
            return (GB_OUT_OF_MEMORY) ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // A is non-hypersparse
        //----------------------------------------------------------------------

        int64_t plen = A->vdim ;
        A->nvec = plen ;
        A->plen = plen ;
        GB_CALLOC_MEMORY (A->p, plen+1, sizeof (int64_t)) ;
        ASSERT (A->h == NULL) ;
        if (A->p == NULL)
        { 
            // out of memory
            GB_PHIX_FREE (A) ;
            return (GB_OUT_OF_MEMORY) ;
        }
    }

    //--------------------------------------------------------------------------
    // return a valid empty matrix
    //--------------------------------------------------------------------------

    A->magic = GB_MAGIC ;
    return (GrB_SUCCESS) ;
}

