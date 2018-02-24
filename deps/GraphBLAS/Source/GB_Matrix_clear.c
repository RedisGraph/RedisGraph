//------------------------------------------------------------------------------
// GB_Matrix_clear: clears the content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The A->x and A->i content is freed and the column pointers A->p are set to
// zero.  This puts the matrix A in the same state it had after GrB_Matrix_new
// (&A, ...).  The dimensions and type of A are not changed.  To change the
// size or type requires a GrB_Matrix_free (&A) followed by another call to
// GrB_Matrix_new (&A, ...) with the new dimensions and type.

// This function is not user-callable.  Use GrB_Matrix_clear instead.

#include "GB.h"

void GB_Matrix_clear        // clear a matrix, type and dimensions unchanged
(
    GrB_Matrix A            // matrix to clear
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // If A->p is a shallow copy then the p array in the other matrix is
    // cleared too.  GraphBLAS itself does not do this, and it does not return
    // matrices to the user with shallow content.  So assert A->p is not
    // shallow.

    ASSERT (A != NULL && A->p != NULL && !A->p_shallow) ;

    // A has been created by GB_new, which has allocated A->p.  It need not
    // have initialized its contents, however (MAGIC2 case)
    ASSERT (A->magic == MAGIC || A->magic == MAGIC2) ;

    // zombies and pending tuples have no effect; about to delete them anyway
    ASSERT (PENDING_OK (A)) ;
    ASSERT (ZOMBIES_OK (A)) ;

    //--------------------------------------------------------------------------
    // clear the content of A
    //--------------------------------------------------------------------------

    // free all but A->p
    GB_Matrix_ixfree (A) ;

    // no more zombies or pending tuples
    ASSERT (!PENDING (A)) ;
    ASSERT (!ZOMBIES (A)) ;

    // Set the column pointers to zero.
    for (int64_t j = 0 ; j <= A->ncols ; j++)
    {
        A->p [j] = 0 ;
    }

    // A is now initialized
    A->magic = MAGIC ;
}

