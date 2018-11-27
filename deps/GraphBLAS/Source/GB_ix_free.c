//------------------------------------------------------------------------------
// GB_ix_free: free A->i, A->x, pending tuples, zombies; A->p, A->h unchanged
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Since A->p and A->h are unchanged, the matrix is still valid (unless it was
// invalid on input).  nnz(A) would report zero, and so would GrB_Matrix_nvals.

#include "GB.h"

GrB_Info GB_ix_free             // free A->i and A->x of a matrix
(
    GrB_Matrix A                // matrix with content to free
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (A == NULL)
    { 
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // free all but A->p and A->h
    //--------------------------------------------------------------------------

    // zombies and pending tuples are about to be deleted
    ASSERT (GB_PENDING_OK (A)) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;

    // free A->i unless it is shallow
    if (!A->i_shallow)
    { 
        GB_FREE_MEMORY (A->i, A->nzmax, sizeof (int64_t)) ;
    }
    A->i = NULL ;
    A->i_shallow = false ;

    // free A->x unless it is shallow
    if (!A->x_shallow)
    { 
        // A->type_size is used since A->type might already be freed, and thus
        // A->type->size cannot be accessed.
        GB_FREE_MEMORY (A->x, A->nzmax, A->type_size) ;
    }
    A->x = NULL ;
    A->x_shallow = false ;

    A->nzmax = 0 ;

    // no zombies remain
    A->nzombies = 0 ;

    // free pending tuples
    GB_pending_free (A) ;

    // remove from the queue, if present; panic if critical section fails
    GB_CRITICAL (GB_queue_remove (A)) ;

    return (GrB_SUCCESS) ;
}

