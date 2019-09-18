//------------------------------------------------------------------------------
// GB_nvec_nonempty: count the number of non-empty vectors
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// All pending tuples are ignored.  If a vector has all zombies it is still
// counted as non-empty.

#include "GB.h"

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

    //--------------------------------------------------------------------------
    // trivial case
    //--------------------------------------------------------------------------

    if (GB_NNZ (A) == 0)
    { 
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
    const int64_t *restrict Ap = A->p ;

    #pragma omp parallel for num_threads(nthreads) schedule(static) \
            reduction(+:nvec_nonempty)
    for (int64_t k = 0 ; k < anvec ; k++)
    { 
        if (Ap [k] < Ap [k+1]) nvec_nonempty++ ;
    }

    ASSERT (nvec_nonempty >= 0 && nvec_nonempty <= A->vdim) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (nvec_nonempty) ;
}

