//------------------------------------------------------------------------------
// GB_convert_full_to_sparse: convert a matrix from full to sparse
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_convert_full_to_sparse      // convert matrix from full to sparse
(
    GrB_Matrix A,               // matrix to convert from full to sparse
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A converting full to sparse", GB0) ;
    ASSERT (GB_IS_FULL (A) || A->nzmax == 0) ;
    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (!GB_IS_SPARSE (A)) ;
    ASSERT (!GB_IS_HYPERSPARSE (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    GBURBLE ("(full to sparse) ") ;

    //--------------------------------------------------------------------------
    // allocate A->p and A->i
    //--------------------------------------------------------------------------

    int64_t avdim = A->vdim ;
    int64_t avlen = A->vlen ;
    int64_t anz = avdim * avlen ;
    ASSERT (GB_Index_multiply (&anz, avdim, avlen) == true) ;

    int64_t *GB_RESTRICT Ap = GB_MALLOC (avdim+1, int64_t) ;
    int64_t *GB_RESTRICT Ai = GB_MALLOC (anz, int64_t) ;

    if (Ap == NULL || Ai == NULL)
    { 
        // out of memory
        GB_FREE (Ap) ;
        GB_FREE (Ai) ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    A->p = Ap ;
    A->i = Ai ;
    A->plen = avdim ;
    A->nvec = avdim ;
    A->nvec_nonempty = (avlen == 0) ? 0 : avdim ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // fill the A->p and A->i pattern
    //--------------------------------------------------------------------------

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k <= avdim ; k++)
    { 
        Ap [k] = k * avlen ;
    }

    int64_t p ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (p = 0 ; p < anz ; p++)
    { 
        Ai [p] = p % avlen ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A converted from full to sparse", GB0) ;
    ASSERT (GB_IS_SPARSE (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    return (GrB_SUCCESS) ;
}

