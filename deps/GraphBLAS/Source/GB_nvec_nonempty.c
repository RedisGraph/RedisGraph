//------------------------------------------------------------------------------
// GB_nvec_nonempty: count the number of non-empty vectors
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// All pending tuples are ignored.  If a vector has all zombies it is still
// counted as non-empty.

#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
int64_t GB_nvec_nonempty        // return # of non-empty vectors
(
    const GrB_Matrix A,         // input matrix to examine
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;
    ASSERT (GB_PENDING_OK (A)) ;

    //--------------------------------------------------------------------------
    // trivial cases
    //--------------------------------------------------------------------------

    if (GB_IS_FULL (A) || GB_IS_BITMAP (A))
    { 
        // A is full or bitmap; nvec_nonempty depends only on the dimensions
        return ((A->vlen == 0) ? 0 : A->vdim) ;
    }

    if (GB_NNZ (A) == 0)
    { 
        // A is sparse or hypersparse, with no entries
        return (0) ;
    }

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t anvec = A->nvec ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anvec, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // count the non-empty columns
    //--------------------------------------------------------------------------

    int64_t nvec_nonempty = 0 ;
    const int64_t *GB_RESTRICT Ap = A->p ;

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static) \
            reduction(+:nvec_nonempty)
    for (k = 0 ; k < anvec ; k++)
    { 
        if (Ap [k] < Ap [k+1]) nvec_nonempty++ ;
    }

    ASSERT (nvec_nonempty >= 0 && nvec_nonempty <= A->vdim) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (nvec_nonempty) ;
}

