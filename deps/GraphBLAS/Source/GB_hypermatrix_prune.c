//------------------------------------------------------------------------------
// GB_hypermatrix_prune: prune empty vectors from a hypersparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_hypermatrix_prune
(
    GrB_Matrix A,               // matrix to prune
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;        // pattern not accessed
    ASSERT (GB_JUMBLED_OK (A)) ;

    if (!GB_IS_HYPERSPARSE (A))
    { 
        // nothing to do
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // count # of empty vectors
    //--------------------------------------------------------------------------

    if (A->nvec_nonempty < 0)
    { 
        A->nvec_nonempty = GB_nvec_nonempty (A, Context) ;
    }

    //--------------------------------------------------------------------------
    // prune empty vectors
    //--------------------------------------------------------------------------

    if (A->nvec_nonempty < A->nvec)     // A->nvec_nonempty used here
    {
        // create new Ap_new and Ah_new arrays, with no empty vectors
        int64_t *GB_RESTRICT Ap_new = NULL ;
        int64_t *GB_RESTRICT Ah_new = NULL ;
        int64_t nvec_new ;
        GrB_Info info = GB_hyper_prune (&Ap_new, &Ah_new, &nvec_new,
            A->p, A->h, A->nvec, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            return (info) ;
        }
        // free the old A->p and A->h (they might be shallow)
        GB_ph_free (A) ;
        // transplant the new hyperlist into A
        A->p = Ap_new ;
        A->h = Ah_new ;
        A->nvec = nvec_new ;
        A->plen = nvec_new ;
        A->nvec_nonempty = nvec_new ;
        A->magic = GB_MAGIC ;
        ASSERT (!A->p_shallow) ;
        ASSERT (!A->h_shallow) ;
    }

    return (GrB_SUCCESS) ;
}

