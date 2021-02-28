//------------------------------------------------------------------------------
// GB_hyper_prune: remove empty vectors from a hypersparse Ap, Ah list
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Removes empty vectors from a hypersparse list.  On input, *Ap and *Ah are
// assumed to be NULL.  The input arrays Ap_old and Ah_old are not modified,
// and thus can be shallow content from another matrix.  New hyperlists Ap and
// Ah are allocated, for nvec vectors, all nonempty.

#include "GB.h"

GrB_Info GB_hyper_prune
(
    // output, not allocated on input:
    int64_t *GB_RESTRICT *p_Ap,        // size nvec+1
    int64_t *GB_RESTRICT *p_Ah,        // size nvec
    int64_t *p_nvec,                // # of vectors, all nonempty
    // input, not modified
    const int64_t *Ap_old,          // size nvec_old+1
    const int64_t *Ah_old,          // size nvec_old
    const int64_t nvec_old,         // original number of vectors
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (p_Ap != NULL) ;
    ASSERT (p_Ah != NULL) ;
    ASSERT (p_nvec != NULL) ;
    ASSERT (Ap_old != NULL) ;
    ASSERT (Ah_old != NULL) ;
    ASSERT (nvec_old >= 0) ;
    (*p_Ap) = NULL ;
    (*p_Ah) = NULL ;
    (*p_nvec) = -1 ;

    //--------------------------------------------------------------------------
    // determine the # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (nvec_old, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT W ;
    GB_MALLOC_MEMORY (W, nvec_old+1, sizeof (int64_t)) ;
    if (W == NULL)
    { 
        // out of memory
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // count the # of nonempty vectors
    //--------------------------------------------------------------------------

    int64_t k ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < nvec_old ; k++)
    { 
        // W [k] = 1 if the kth vector is nonempty; 0 if empty
        W [k] = (Ap_old [k] < Ap_old [k+1]) ;
    }

    int64_t nvec ;
    GB_cumsum (W, nvec_old, &nvec, nthreads) ;

    //--------------------------------------------------------------------------
    // allocate the result
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Ap = NULL ;
    int64_t *GB_RESTRICT Ah = NULL ;
    GB_MALLOC_MEMORY (Ap, nvec+1, sizeof (int64_t)) ;
    GB_MALLOC_MEMORY (Ah, nvec,   sizeof (int64_t)) ;
    if (Ap == NULL || Ah == NULL)
    { 
        // out of memory
        GB_FREE_MEMORY (W, nvec_old+1, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Ap, nvec+1, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Ah, nvec,   sizeof (int64_t)) ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // create the Ap and Ah result
    //--------------------------------------------------------------------------

    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (k = 0 ; k < nvec_old ; k++)
    {
        if (Ap_old [k] < Ap_old [k+1])
        { 
            int64_t knew = W [k] ;
            Ap [knew] = Ap_old [k] ;
            Ah [knew] = Ah_old [k] ;
        }
    }

    Ap [nvec] = Ap_old [nvec_old] ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_MEMORY (W, nvec_old+1, sizeof (int64_t)) ;
    (*p_Ap) = Ap ;
    (*p_Ah) = Ah ;
    (*p_nvec) = nvec ;
    return (GrB_SUCCESS) ;
}

