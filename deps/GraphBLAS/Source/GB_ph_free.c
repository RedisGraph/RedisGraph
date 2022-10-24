//------------------------------------------------------------------------------
// GB_ph_free: free the A->p and A->h content of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Free the A->p and A->h content of a matrix.  The matrix becomes invalid, and
// would generate a GrB_INVALID_OBJECT error if passed to a user-callable
// GraphBLAS function.

#include "GB.h"

GB_PUBLIC
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
    if (!A->p_shallow)
    { 
        GB_FREE (&(A->p), A->p_size) ;
    }
    A->p = NULL ;
    A->p_size = 0 ;
    A->p_shallow = false ;

    // free A->h unless it is shallow
    if (!A->h_shallow)
    { 
        GB_FREE (&(A->h), A->h_size) ;
    }
    A->h = NULL ;
    A->h_size = 0 ;
    A->h_shallow = false ;

    A->plen = 0 ;
    A->nvec = 0 ;
    A->nvec_nonempty = 0 ;

    //--------------------------------------------------------------------------
    // set the status to invalid
    //--------------------------------------------------------------------------

    // If this matrix is used as input to a user-callable GraphBLAS function,
    // it will generate an error: GrB_INVALID_OBJECT.
    A->magic = GB_MAGIC2 ;
}

