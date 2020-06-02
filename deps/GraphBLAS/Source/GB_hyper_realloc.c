//------------------------------------------------------------------------------
// GB_hyper_realloc: reallocate a matrix hyperlist
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Change the size of the A->h and A->p hyperlist.
// No change is made if A is not hypersparse.

#include "GB.h"

GrB_Info GB_hyper_realloc
(
    GrB_Matrix A,               // matrix with hyperlist to reallocate
    int64_t plen_new,           // new size of A->p and A->h
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;
    ASSERT (!A->p_shallow) ;
    ASSERT (!A->h_shallow) ;
    ASSERT (A->p != NULL) ;

    //--------------------------------------------------------------------------
    // reallocate the hyperlist
    //--------------------------------------------------------------------------

    if (A->is_hyper)
    {

        ASSERT (A->h != NULL) ;

        // old size of A->p and A->h
        int64_t plen_old = A->plen ;

        // change the size of A->h and A->p
        bool ok1 = true, ok2 = true ;
        GB_REALLOC_MEMORY (A->p, plen_new+1, plen_old+1, sizeof(int64_t), &ok1);
        GB_REALLOC_MEMORY (A->h, plen_new,   plen_old,   sizeof(int64_t), &ok2);
        bool ok = ok1 && ok2 ;

        // always succeeds if the space shrinks
        ASSERT (GB_IMPLIES (plen_new <= plen_old, ok)) ;

        if (!ok)
        { 
            // out of memory
            GB_PHIX_FREE (A) ;
            return (GB_OUT_OF_MEMORY) ;
        }

        // size of A->p and A->h has been changed
        A->plen = plen_new ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

