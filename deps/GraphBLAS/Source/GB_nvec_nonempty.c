//------------------------------------------------------------------------------
// GB_nvec_nonempty: count the number of non-empty vectors
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Pending tuples are ignored.  If a vector has all zombies it is still
// counted as non-empty.

// PARALLEL: simple parallel reduction

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

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;

    //--------------------------------------------------------------------------
    // trivial case
    //--------------------------------------------------------------------------

    if (GB_NNZ (A) == 0)
    { 
        return (0) ;
    }

    //--------------------------------------------------------------------------
    // count the non-empty columns
    //--------------------------------------------------------------------------

    int64_t nvec_nonempty = 0 ;

    GBI_for_each_vector (A)
    { 
        GBI_jth_iteration (j, p, pend) ;
        int64_t ajnz = pend - p ;
        if (ajnz > 0) nvec_nonempty++ ;
    }

    ASSERT (nvec_nonempty >= 0 && nvec_nonempty <= A->vdim) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (nvec_nonempty) ;
}

