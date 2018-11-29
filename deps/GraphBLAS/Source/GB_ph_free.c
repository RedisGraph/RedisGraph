//------------------------------------------------------------------------------
// GB_ph_free: free the A->p and A->h content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Free the A->p and A->h content of a matrix.  If followed by GB_ix_free(A),
// the header of A is just like GB_new with GB_Ap_null.  No content is left
// except the header.  The matrix becomes invalid, and would generate a
// GrB_INVALID_OBJECT error if passed to a user-callable GraphBLAS function.

#include "GB.h"

void GB_ph_free                 // free A->p and A->h of a matrix
(
    GrB_Matrix A                // matrix with content to free
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (A == NULL)
    { 
        return ;
    }

    //--------------------------------------------------------------------------
    // free A->p and A->h
    //--------------------------------------------------------------------------

    // free A->p unless it is shallow
    if (!A->p_shallow) GB_FREE_MEMORY (A->p, A->plen+1, sizeof (int64_t)) ;
    A->p = NULL ;
    A->p_shallow = false ;

    // free A->h unless it is shallow
    if (!A->h_shallow) GB_FREE_MEMORY (A->h, A->plen,   sizeof (int64_t)) ;
    A->h = NULL ;
    A->h_shallow = false ;

    if (A->is_hyper)
    { 
        A->plen = 0 ;
        A->nvec = 0 ;
    }

    A->nvec_nonempty = 0 ;

    //--------------------------------------------------------------------------
    // set the status to invalid
    //--------------------------------------------------------------------------

    // If this matrix is used as input to a user-callable GraphBLAS function,
    // it will generate an error: GrB_INVALID_OBJECT.
    A->magic = GB_MAGIC2 ;
}

