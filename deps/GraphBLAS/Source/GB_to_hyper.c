//------------------------------------------------------------------------------
// GB_to_hyper: convert a matrix to hyperspasre
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// On input, the matrix may have shallow A->p content; it is safely removed.
// On output, the matrix is always hypersparse (even if out of memory).  If the
// input matrix is non-hypersparse, it is given new A->p and A->h that are not
// shallow.  If the input matrix is already hypersparse, nothing is changed
// (and in that case A->p and A->h remain shallow on output if shallow on
// input). The A->x and A->i content is not changed; it remains in whatever
// shallow/non-shallow state that it had on input).

// A->nvec_nonempty does not change.

// If an out-of-memory condition occurs, all content of the matrix is cleared.

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

    // GB_subref_numeric can return a matrix with jumbled columns, since it
    // soon be transposed (and sorted) in GB_accum_mask.  However, it passes
    // the jumbled matrix to GB_to_hyper_conform.  This function does not
    // access the row indices at all, so it works fine if the columns have
    // jumbled row indices.

    ASSERT_OK_OR_JUMBLED (GB_check (A, "A converting to hypersparse", GB0)) ;

    #ifndef NDEBUG
    GrB_Info info ;
    #endif

    //--------------------------------------------------------------------------
    // convert A to hypersparse form
    //--------------------------------------------------------------------------

    if (!A->is_hyper)
    {
        ASSERT (A->h == NULL) ;
        ASSERT (A->nvec == A->plen && A->plen == A->vdim) ;

        //----------------------------------------------------------------------
        // count the number of non-empty vectors in A
        //----------------------------------------------------------------------

        int64_t *restrict Ap_old = A->p ;
        bool Ap_old_shallow = A->p_shallow ;

        int64_t n = A->vdim ;
        int64_t nvec_new = A->nvec_nonempty ;

        //----------------------------------------------------------------------
        // allocate the new A->p and A->h
        //----------------------------------------------------------------------

        int64_t *restrict Ap_new ;
        int64_t *restrict Ah_new ;
        GB_MALLOC_MEMORY (Ap_new, nvec_new+1, sizeof (int64_t)) ;
        GB_MALLOC_MEMORY (Ah_new, nvec_new,   sizeof (int64_t)) ;
        if (Ap_new == NULL || Ah_new == NULL)
        { 
            // out of memory
            A->is_hyper = true ;    // A is hypersparse, but otherwise invalid
            GB_FREE_MEMORY (Ap_new, nvec_new+1, sizeof (int64_t)) ;
            GB_FREE_MEMORY (Ah_new, nvec_new,   sizeof (int64_t)) ;
            GB_CONTENT_FREE (A) ;
            return (GB_OUT_OF_MEMORY (GBYTES (2*nvec_new+1, sizeof (int64_t))));
        }

        //----------------------------------------------------------------------
        // transplant the new A->p and A->h into the matrix
        //----------------------------------------------------------------------

        // this must be done here so that GB_jappend, just below, can be used.

        A->is_hyper = true ;
        A->plen = nvec_new ;
        A->nvec = 0 ;

        A->p = Ap_new ;
        A->h = Ah_new ;
        A->p_shallow = false ;
        A->h_shallow = false ;

        //----------------------------------------------------------------------
        // construct the new hyperlist in the new A->p and A->h
        //----------------------------------------------------------------------

        int64_t jlast, anz, anz_last ;
        GB_jstartup (A, &jlast, &anz, &anz_last) ;

        for (int64_t j = 0 ; j < n ; j++)
        { 
            anz = Ap_old [j+1] ;
            ASSERT (A->nvec <= A->plen) ;
            #ifndef NDEBUG
            info = 
            #endif
            GB_jappend (A, j, &jlast, anz, &anz_last, Context) ;
            ASSERT (info == GrB_SUCCESS) ;
            ASSERT (A->nvec <= A->plen) ;
        }
        GB_jwrapup (A, jlast, anz) ;
        ASSERT (A->nvec == nvec_new) ;
        ASSERT (A->nvec_nonempty == nvec_new) ;

        //----------------------------------------------------------------------
        // free the old A->p unless it's shallow
        //----------------------------------------------------------------------

        // this cannot use GB_ph_free because the new A->p content has already
        // been placed into A, as required by GB_jappend just above.

        if (!Ap_old_shallow)
        { 
            GB_FREE_MEMORY (Ap_old, n+1, sizeof (int64_t)) ;
        }
    }

    //--------------------------------------------------------------------------
    // A is now in hypersparse form
    //--------------------------------------------------------------------------

    ASSERT_OK_OR_JUMBLED (GB_check (A, "A converted to hypersparse", GB0)) ;
    ASSERT (A->is_hyper) ;
    return (GrB_SUCCESS) ;
}

