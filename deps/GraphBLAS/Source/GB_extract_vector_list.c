//------------------------------------------------------------------------------
// GB_extract_vector_list: extract vector indices for all entries in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Constructs a list of vector indices for each entry in a matrix.  Creates
// the output J for GB_extractTuples, and I for GB_transpose when the qsort
// method is used.

#include "GB_ek_slice.h"

#define GB_FREE_WORK \
    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice) ;

bool GB_extract_vector_list     // true if successful, false if out of memory
(
    // output:
    int64_t *GB_RESTRICT J,        // size nnz(A) or more
    // input:
    const GrB_Matrix A,
    int nthreads
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (J != NULL) ;
    ASSERT (A != NULL) ;
    ASSERT (nthreads >= 1) ;
    ASSERT (GB_JUMBLED_OK (A)) ;        // pattern not accessed
    ASSERT (GB_ZOMBIES_OK (A)) ;        // pattern not accessed
    ASSERT (!GB_IS_BITMAP (A)) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int64_t avlen = A->vlen ;

    //--------------------------------------------------------------------------
    // determine the # of tasks to use
    //--------------------------------------------------------------------------

    int ntasks = (nthreads == 1) ? 1 : (2 * nthreads) ;

    // TODO: use #include "GB_positional_op_ijp.c" here

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    // Task tid does entries pstart_slice [tid] to pstart_slice [tid+1]-1 and
    // vectors kfirst_slice [tid] to klast_slice [tid].  The first and last
    // vectors may be shared with prior slices and subsequent slices.

    int64_t *pstart_slice = NULL, *kfirst_slice = NULL, *klast_slice = NULL ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, A, &ntasks))
    { 
        // out of memory
        return (false) ;
    }

    //--------------------------------------------------------------------------
    // extract the vector index for each entry
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        // if kfirst > klast then task tid does no work at all
        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) to be operated on by this task
            //------------------------------------------------------------------

            int64_t j = GBH (Ah, k) ;
            int64_t pA_start, pA_end ;
            GB_get_pA (&pA_start, &pA_end, tid, k, 
                kfirst, klast, pstart_slice, Ap, avlen) ;

            //------------------------------------------------------------------
            // extract vector indices of A(:,j)
            //------------------------------------------------------------------

            for (int64_t p = pA_start ; p < pA_end ; p++)
            { 
                J [p] = j ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    return (true) ;
}

