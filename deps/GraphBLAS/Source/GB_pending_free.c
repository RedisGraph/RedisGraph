//------------------------------------------------------------------------------
// GB_pending_free: free all pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

void GB_pending_free            // free all pending tuples
(
    GrB_Matrix A                // matrix with pending tuples to free
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;

    //--------------------------------------------------------------------------
    // free all pending tuples
    //--------------------------------------------------------------------------

    // If A->type_pending is NULL, A->s_pending must be NULL too, but free it
    // anyway just in case it is not NULL.

    GB_FREE_MEMORY (A->i_pending, A->max_n_pending, sizeof (int64_t)) ;
    GB_FREE_MEMORY (A->j_pending, A->max_n_pending, sizeof (int64_t)) ;
    GB_FREE_MEMORY (A->s_pending, A->max_n_pending, A->type_pending_size) ;

    A->n_pending = 0 ;
    A->max_n_pending = 0 ;
    A->sorted_pending = true ;
    A->operator_pending = NULL ;
    A->type_pending = NULL ;
    A->type_pending_size = 0 ;
}

