//------------------------------------------------------------------------------
// GB_to_hyper: convert a matrix to hyperspasre
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// On input, the matrix may have shallow A->p content; it is safely removed.
// On output, the matrix is always hypersparse (even if out of memory).  If the
// input matrix is non-hypersparse, it is given new A->p and A->h that are not
// shallow.  If the input matrix is already hypersparse, nothing is changed
// (and in that case A->p and A->h remain shallow on output if shallow on
// input). The A->x and A->i content is not changed; it remains in whatever
// shallow/non-shallow state that it had on input).

// If an out-of-memory condition occurs, all content of the matrix is cleared.

// The input matrix may be jumbled; this is not an error condition.

#include "GB.h"

GrB_Info GB_to_hyper        // convert a matrix to hypersparse
(
    GrB_Matrix A,           // matrix to convert to hypersparse
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK_OR_JUMBLED (A, "A converting to hypersparse", GB0) ;
    int64_t anz = GB_NNZ (A) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;

    //--------------------------------------------------------------------------
    // convert A to hypersparse form
    //--------------------------------------------------------------------------

    if (!A->is_hyper)
    {

        //----------------------------------------------------------------------
        // determine the number of threads to use
        //----------------------------------------------------------------------

        int64_t n = A->vdim ;
        GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
        int nthreads = GB_nthreads (n, chunk, nthreads_max) ;
        int ntasks = (nthreads == 1) ? 1 : (8 * nthreads) ;
        ntasks = GB_IMIN (ntasks, n) ;
        ntasks = GB_IMAX (ntasks, 1) ;

        //----------------------------------------------------------------------
        // count the number of non-empty vectors in A in each slice
        //----------------------------------------------------------------------

        A->is_hyper = true ;    // A becomes hypersparse
        ASSERT (A->h == NULL) ;
        ASSERT (A->nvec == A->plen && A->plen == n) ;

        const int64_t *GB_RESTRICT Ap_old = A->p ;
        bool Ap_old_shallow = A->p_shallow ;

        int64_t *GB_RESTRICT Count ;
        GB_MALLOC_MEMORY (Count, ntasks+1, sizeof (int64_t)) ;
        if (Count == NULL)
        { 
            // out of memory
            GB_PHIX_FREE (A) ;
            return (GB_OUT_OF_MEMORY) ;
        }

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        for (tid = 0 ; tid < ntasks ; tid++)
        {
            int64_t jstart, jend, my_nvec_nonempty = 0 ; ;
            GB_PARTITION (jstart, jend, n, tid, ntasks) ;
            for (int64_t j = jstart ; j < jend ; j++)
            { 
                if (Ap_old [j] < Ap_old [j+1]) my_nvec_nonempty++ ;
            }
            Count [tid] = my_nvec_nonempty ;
        }

        //----------------------------------------------------------------------
        // compute cumulative sum of Counts and nvec_nonempty
        //----------------------------------------------------------------------

        GB_cumsum (Count, ntasks, NULL, 1) ;
        int64_t nvec_nonempty = Count [ntasks] ;
        A->nvec_nonempty = nvec_nonempty ;

        //----------------------------------------------------------------------
        // allocate the new A->p and A->h
        //----------------------------------------------------------------------

        int64_t *GB_RESTRICT Ap_new ;
        int64_t *GB_RESTRICT Ah_new ;
        GB_MALLOC_MEMORY (Ap_new, nvec_nonempty+1, sizeof (int64_t)) ;
        GB_MALLOC_MEMORY (Ah_new, nvec_nonempty,   sizeof (int64_t)) ;
        if (Ap_new == NULL || Ah_new == NULL)
        { 
            // out of memory
            GB_FREE_MEMORY (Count, ntasks+1, sizeof (int64_t)) ;
            GB_FREE_MEMORY (Ap_new, nvec_nonempty+1, sizeof (int64_t)) ;
            GB_FREE_MEMORY (Ah_new, nvec_nonempty,   sizeof (int64_t)) ;
            GB_PHIX_FREE (A) ;
            return (GB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // transplant the new A->p and A->h into the matrix
        //----------------------------------------------------------------------

        A->plen = nvec_nonempty ;
        A->nvec = nvec_nonempty ;
        A->p = Ap_new ;
        A->h = Ah_new ;
        A->p_shallow = false ;
        A->h_shallow = false ;

        //----------------------------------------------------------------------
        // construct the new hyperlist in the new A->p and A->h
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        for (tid = 0 ; tid < ntasks ; tid++)
        {
            int64_t jstart, jend, k = Count [tid] ;
            GB_PARTITION (jstart, jend, n, tid, ntasks) ;
            for (int64_t j = jstart ; j < jend ; j++)
            {
                if (Ap_old [j] < Ap_old [j+1])
                { 
                    // vector index j is the kth vector in the new Ah
                    Ap_new [k] = Ap_old [j] ;
                    Ah_new [k] = j ;
                    k++ ;
                }
            }
            ASSERT (k == Count [tid+1]) ;
        }

        Ap_new [nvec_nonempty] = anz ;
        A->magic = GB_MAGIC ;
        ASSERT (A->nvec_nonempty == GB_nvec_nonempty (A, Context)) ;

        //----------------------------------------------------------------------
        // free workspace, and free the old A->p unless it's shallow
        //----------------------------------------------------------------------

        GB_FREE_MEMORY (Count, ntasks+1, sizeof (int64_t)) ;
        if (!Ap_old_shallow)
        { 
            GB_FREE_MEMORY (Ap_old, n+1, sizeof (int64_t)) ;
        }
    }

    //--------------------------------------------------------------------------
    // A is now in hypersparse form
    //--------------------------------------------------------------------------

    ASSERT (anz == GB_NNZ (A)) ;
    ASSERT_MATRIX_OK_OR_JUMBLED (A, "A converted to hypersparse", GB0) ;
    ASSERT (A->is_hyper) ;
    return (GrB_SUCCESS) ;
}

