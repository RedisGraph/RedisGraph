//------------------------------------------------------------------------------
// GB_ijsort:  sort an index array I and remove duplicates
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Sort an index array and remove duplicates.  In MATLAB notation:

/*
    [I1 I1k] = sort (I) ;
    Iduplicate = [I1 (1:end-1) == I1 (2:end)), false] ;
    I2  = I1  (~Iduplicate) ;
    I2k = I1k (~Iduplicate) ;
*/

#include "GB_ij.h"
#include "GB_sort.h"

#define GB_FREE_WORK                                    \
{                                                       \
    GB_FREE_MEMORY (W0,  ni, sizeof (GrB_Index)) ;      \
    GB_FREE_MEMORY (W1,  ni, sizeof (GrB_Index)) ;      \
    GB_FREE_MEMORY (I1,  ni, sizeof (GrB_Index)) ;      \
    GB_FREE_MEMORY (I1k, ni, sizeof (GrB_Index)) ;      \
}

GrB_Info GB_ijsort
(
    const GrB_Index *restrict I, // size ni, where ni > 1 always holds
    int64_t *restrict p_ni,      // : size of I, output: # of indices in I2
    GrB_Index *restrict *p_I2,   // size ni2, where I2 [0..ni2-1]
                        // contains the sorted indices with duplicates removed.
    GrB_Index *restrict *p_I2k,  // output array of size ni2
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (I != NULL) ;
    ASSERT (p_ni != NULL) ;
    ASSERT (p_I2 != NULL) ;
    ASSERT (p_I2k != NULL) ;

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GrB_Index *restrict I1  = NULL ;
    GrB_Index *restrict I1k = NULL ;
    GrB_Index *restrict I2  = NULL ;
    GrB_Index *restrict I2k = NULL ;
    int64_t *restrict W0  = NULL ;
    int64_t *restrict W1 = NULL ;
    int64_t ni = *p_ni ;
    ASSERT (ni > 1) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (ni, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    GB_MALLOC_MEMORY (I1,  ni, sizeof (GrB_Index)) ;
    GB_MALLOC_MEMORY (I1k, ni, sizeof (GrB_Index)) ;
    if (I1 == NULL || I1k == NULL)
    { 
        // out of memory
        GB_FREE_WORK ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // copy I into I1 and construct I1k
    //--------------------------------------------------------------------------

    GB_memcpy (I1, I, ni * sizeof (GrB_Index), nthreads) ;

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (int64_t k = 0 ; k < ni ; k++)
    { 
        I1k [k] = k ;
    }

    //--------------------------------------------------------------------------
    // sort [I1 I1k]
    //--------------------------------------------------------------------------

    if (nthreads == 1)
    { 

        //----------------------------------------------------------------------
        // sequential quicksort
        //----------------------------------------------------------------------

        GB_qsort_2 ((int64_t *) I1, (int64_t *) I1k, ni) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // parallel mergesort
        //----------------------------------------------------------------------

        GB_MALLOC_MEMORY (W0, ni, sizeof (int64_t)) ;
        GB_MALLOC_MEMORY (W1, ni, sizeof (int64_t)) ;
        if (W0 == NULL || W1 == NULL)
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GB_OUT_OF_MEMORY) ;
        }

        GB_msort_2 ((int64_t *) I1, (int64_t *) I1k, W0, W1, ni, nthreads) ;

        GB_FREE_MEMORY (W0, ni, sizeof (int64_t)) ;
        GB_FREE_MEMORY (W1, ni, sizeof (int64_t)) ;
    }

    //--------------------------------------------------------------------------
    // count unique entries in I1
    //--------------------------------------------------------------------------

    int ntasks = (nthreads == 1) ? 1 : (32 * nthreads) ;
    ntasks = GB_IMIN (ntasks, ni) ;
    ntasks = GB_IMAX (ntasks, 1) ;

    int64_t Count [ntasks+1] ;

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (int tid = 0 ; tid < ntasks ; tid++)
    {
        int64_t kfirst, klast, my_count = (tid == 0) ? 1 : 0 ;
        GB_PARTITION (kfirst, klast, ni, tid, ntasks) ;
        for (int64_t k = GB_IMAX (kfirst,1) ; k < klast ; k++)
        {
            if (I1 [k-1] != I1 [k])
            { 
                my_count++ ;
            }
        }
        Count [tid] = my_count ;
    }

    GB_cumsum (Count, ntasks, NULL, 1) ;
    int64_t ni2 = Count [ntasks] ;

    //--------------------------------------------------------------------------
    // allocate the result I2
    //--------------------------------------------------------------------------

    GB_MALLOC_MEMORY (I2 , ni2, sizeof (GrB_Index)) ;
    GB_MALLOC_MEMORY (I2k, ni2, sizeof (GrB_Index)) ;
    if (I2 == NULL || I2k == NULL)
    { 
        // out of memory
        GB_FREE_WORK ;
        GB_FREE_MEMORY (I2 , ni2, sizeof (GrB_Index)) ;
        GB_FREE_MEMORY (I2k, ni2, sizeof (GrB_Index)) ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // construct the new list I2 from I1, removing duplicates
    //--------------------------------------------------------------------------

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (int tid = 0 ; tid < ntasks ; tid++)
    {
        int64_t kfirst, klast, k2 = Count [tid] ;
        GB_PARTITION (kfirst, klast, ni, tid, ntasks) ;
        if (tid == 0)
        { 
            // the first entry in I1 is never a duplicate
            I2  [k2] = I1  [0] ;
            I2k [k2] = I1k [0] ;
            k2++ ;
        }
        for (int64_t k = GB_IMAX (kfirst,1) ; k < klast ; k++)
        {
            if (I1 [k-1] != I1 [k])
            { 
                I2  [k2] = I1  [k] ;
                I2k [k2] = I1k [k] ;
                k2++ ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // check result: compare with single-pass, single-threaded algorithm
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    {
        int64_t ni1 = 1 ;
        for (int64_t k = 1 ; k < ni ; k++)
        {
            if (I1 [ni1-1] != I1 [k])
            {
                I1  [ni1] = I1  [k] ;
                I1k [ni1] = I1k [k] ;
                ni1++ ;
            }
        }
        // printf ("OK "GBd" "GBd"\n", ni1, ni) ;
        ASSERT (ni1 == ni2) ;
        for (int64_t k = 0 ; k < ni1 ; k++)
        {
            ASSERT (I1  [k] == I2 [k]) ;
            ASSERT (I1k [k] == I2k [k]) ;
        }
    }
    #endif

    //--------------------------------------------------------------------------
    // free workspace and return the new sorted list
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    *(p_I2 ) = (GrB_Index *) I2 ;
    *(p_I2k) = (GrB_Index *) I2k ;
    *(p_ni ) = (int64_t    ) ni2 ;

    return (GrB_SUCCESS) ;
}

