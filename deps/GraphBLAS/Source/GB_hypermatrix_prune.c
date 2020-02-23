//------------------------------------------------------------------------------
// GB_hypermatrix_prune: prune empty vectors from a hypersparse matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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
    ASSERT (GB_ZOMBIES_OK (A)) ;
    if (!A->is_hyper)
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

    if (A->nvec_nonempty < A->nvec)
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
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT (A->nvec_nonempty == GB_nvec_nonempty (A, Context)) ;
    return (GrB_SUCCESS) ;
}

