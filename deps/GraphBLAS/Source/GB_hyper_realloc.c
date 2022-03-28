//------------------------------------------------------------------------------
// GB_hyper_realloc: reallocate a matrix hyperlist
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

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
    ASSERT (GB_ZOMBIES_OK (A)) ;        // pattern not accessed
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_PENDING_OK (A)) ;

    //--------------------------------------------------------------------------
    // reallocate the hyperlist
    //--------------------------------------------------------------------------

    if (GB_IS_HYPERSPARSE (A))
    {
        ASSERT (!A->p_shallow) ;
        ASSERT (!A->h_shallow) ;

        // old size of A->p and A->h
        int64_t plen_old = A->plen ;

        // change the size of A->h and A->p
        bool ok1 = true, ok2 = true ;
        GB_REALLOC (A->p, plen_new+1, int64_t, &(A->p_size), &ok1, Context) ;
        GB_REALLOC (A->h, plen_new,   int64_t, &(A->h_size), &ok2, Context) ;
        bool ok = ok1 && ok2 ;

        // always succeeds if the space shrinks
        ASSERT (GB_IMPLIES (plen_new <= plen_old, ok)) ;

        if (!ok)
        { 
            // out of memory
            GB_phbix_free (A) ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        // size of A->p and A->h has been changed
        A->plen = plen_new ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

