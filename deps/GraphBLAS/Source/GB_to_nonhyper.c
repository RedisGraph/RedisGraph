//------------------------------------------------------------------------------
// GB_to_nonhyper: convert a matrix to non-hypersparse form
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// On input, the matrix may have shallow A->p and A->h content; it is safely
// removed.  On output, the matrix is always non-hypersparse (even if out of
// memory).  If the input matrix is hypersparse, it is given a new A->p that is
// not shallow.  If the input matrix is already non-hypersparse, nothing is
// changed (and in that case A->p remains shallow on output if shallow on
// input). The A->x and A->i content is not changed; it remains in whatever
// shallow/non-shallow state that it had on input).

// A->nvec_nonempty does not change.

// If an out-of-memory condition occurs, all content of the matrix is cleared.

#include "GB.h"

GrB_Info GB_to_nonhyper     // convert a matrix to non-hypersparse
(
    GrB_Matrix A,           // matrix to convert to non-hypersparse
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // GB_subref_numeric can return a matrix with jumbled columns, since it
    // soon be transposed (and sorted) in GB_accum_mask.  However, it passes
    // the jumbled matrix to GB_to_hyper_conform.  This function does not
    // access the row indices at all, so it works fine if the columns have
    // jumbled row indices.

    ASSERT_OK_OR_JUMBLED (GB_check (A, "A being converted to nonhyper", GB0)) ;

    //--------------------------------------------------------------------------
    // convert A to non-hypersparse form
    //--------------------------------------------------------------------------

    if (A->is_hyper)
    {

        // allocate the new Ap array, of size A->vdim+1
        int64_t vdim = A->vdim ;
        int64_t *restrict Ap_new ;
        GB_MALLOC_MEMORY (Ap_new, vdim+1, sizeof (int64_t)) ;
        if (Ap_new == NULL)
        { 
            // out of memory
            A->is_hyper = false ;    // A is non-hypersparse, but invalid
            GB_CONTENT_FREE (A) ;
            return (GB_OUT_OF_MEMORY (GBYTES (vdim+1, sizeof (int64_t)))) ;
        }

        // get the old hyperlist
        int64_t *restrict Ap_old = A->p ;
        int64_t *restrict Ah_old = A->h ;
        bool Ap_old_shallow = A->p_shallow ;
        bool Ah_old_shallow = A->h_shallow ;
        int64_t nvec = A->nvec ;
        ASSERT (A->nvec_nonempty == nvec) ;

        // construct the new vector pointers
        int64_t jlast = -1 ;
        for (int64_t k = 0 ; k < nvec ; k++)
        {
            // j is the kth hypersparse vector, and is currently at Ap_old [k]
            // ... Ap_old [k+1]-1.  Vectors jlast+1 to j-1 are all empty and
            // need to have vector pointers in Ap_new all equal to Ap_old [k],
            // and Ap_new [j] also needs to be equal to Ap_old [k].
            int64_t j = Ah_old [k] ;
            ASSERT (j > jlast && j < vdim) ;
            int64_t p = Ap_old [k] ;
            for (int64_t jprior = jlast+1 ; jprior <= j ; jprior++)
            { 
                // mark the start of vectors jlast+1 to j
                Ap_new [jprior] = p ;
            }
            jlast = j ;
        }

        // mark the end of vectors jlast to vdim-1
        int64_t anz = Ap_old [nvec] ;
        for (int64_t jprior = jlast+1 ; jprior <= vdim ; jprior++)
        { 
            // mark the start of vectors jlast+1 to j
            Ap_new [jprior] = anz ;
        }

        // free the old A->p and A->h hyperlist content.
        // this clears A->nvec_nonempty so it must be restored below.
        GB_ph_free (A) ;

        // transplant the new vector pointers; matrix is no longer hypersparse
        A->p = Ap_new ;
        A->h = NULL ;
        A->is_hyper = false ;
        A->nvec = vdim ;
        A->nvec_nonempty = nvec ;
        A->plen = vdim ;
        A->p_shallow = false ;
        A->h_shallow = false ;
        A->magic = GB_MAGIC ;
    }

    //--------------------------------------------------------------------------
    // A is now in non-hypersparse form
    //--------------------------------------------------------------------------

    ASSERT_OK_OR_JUMBLED (GB_check (A, "A converted to nonhypersparse", GB0)) ;
    ASSERT (!(A->is_hyper)) ;
    return (GrB_SUCCESS) ;
}

