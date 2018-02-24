//------------------------------------------------------------------------------
// GB_free_pending: free all pending tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

void GB_free_pending            // free all pending tuples
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

    GB_FREE_MEMORY (A->ipending, A->max_npending, sizeof (int64_t)) ;
    GB_FREE_MEMORY (A->jpending, A->max_npending, sizeof (int64_t)) ;
    GB_FREE_MEMORY (A->xpending, A->max_npending, A->type->size) ;
    A->npending = 0 ;
    A->max_npending = 0 ;
    A->sorted_pending = true ;
    A->operator_pending = NULL ;
}

