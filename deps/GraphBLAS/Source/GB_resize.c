//------------------------------------------------------------------------------
// GB_resize: change the size of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_select.h"

#define GB_FREE_ALL GB_PHIX_FREE (A) ;

GrB_Info GB_resize              // change the size of a matrix
(
    GrB_Matrix A,               // matrix to modify
    const GrB_Index nrows_new,  // new number of rows in matrix
    const GrB_Index ncols_new,  // new number of columns in matrix
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A to resize", GB0) ;

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format
    //--------------------------------------------------------------------------

    int64_t vdim_old = A->vdim ;
    int64_t vlen_old = A->vlen ;
    int64_t vlen_new, vdim_new ;
    if (A->is_csc)
    { 
        vlen_new = nrows_new ;
        vdim_new = ncols_new ;
    }
    else
    { 
        vlen_new = ncols_new ;
        vdim_new = nrows_new ;
    }

    //--------------------------------------------------------------------------
    // determine the max # of threads to use here
    //--------------------------------------------------------------------------

    // GB_selector (RESIZE) will use a different # of threads

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (vdim_new - vdim_old, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    // only do so if either dimension is shrinking, or if pending tuples exist
    // and vdim_old <= 1 and vdim_new > 1, since in that case, Pending->j has
    // not been allocated yet, but would be required in the resized matrix.

    if (vdim_new < vdim_old || vlen_new < vlen_old ||
        (GB_PENDING (A) && vdim_old <= 1 && vdim_new > 1))
    { 
        GB_WAIT (A) ;
        ASSERT_MATRIX_OK (A, "A to resize, wait", GB0) ;
    }

    //--------------------------------------------------------------------------
    // check for early conversion to hypersparse
    //--------------------------------------------------------------------------

    // If the # of vectors grows very large, it is costly to reallocate enough
    // space for the non-hypersparse A->p component.  So convert the matrix to
    // hypersparse if that happens.

    GrB_Info info ;

    if (A->nvec_nonempty < 0)
    { 
        A->nvec_nonempty = GB_nvec_nonempty (A, Context) ;
    }

    if (GB_to_hyper_test (A, A->nvec_nonempty, vdim_new))
    { 
        GB_OK (GB_to_hyper (A, Context)) ;
    }

    //--------------------------------------------------------------------------
    // resize the number of sparse vectors
    //--------------------------------------------------------------------------

    bool ok = true ;

    int64_t *GB_RESTRICT Ah = A->h ;
    int64_t *GB_RESTRICT Ap = A->p ;
    A->vdim = vdim_new ;

    if (A->is_hyper)
    {

        //----------------------------------------------------------------------
        // A is hypersparse: decrease size of A->p and A->h only if needed
        //----------------------------------------------------------------------

        if (vdim_new < A->plen)
        { 
            // reduce the size of A->p and A->h; this cannot fail
            info = GB_hyper_realloc (A, vdim_new, Context) ;
            ASSERT (info == GrB_SUCCESS) ;
            Ap = A->p ;
            Ah = A->h ;
        }
        if (vdim_new < vdim_old)
        { 
            // descrease A->nvec to delete the vectors outside the range
            // 0...vdim_new-1.
            int64_t pleft = 0 ;
            int64_t pright = GB_IMIN (A->nvec, vdim_new) - 1 ;
            bool found ;
            GB_SPLIT_BINARY_SEARCH (vdim_new, Ah, pleft, pright, found) ;
            A->nvec = pleft ;
        }
    }
    else
    {

        //----------------------------------------------------------------------
        // A is not hypersparse: change size of A->p to match the new vdim
        //----------------------------------------------------------------------

        if (vdim_new != vdim_old)
        {
            // change the size of A->p
            GB_REALLOC_MEMORY (A->p, vdim_new+1, vdim_old+1, sizeof (int64_t),
                &ok) ;
            if (!ok)
            { 
                // out of memory
                GB_FREE_ALL ;
                return (GB_OUT_OF_MEMORY) ;
            }
            Ap = A->p ;
            A->plen = vdim_new ;
        }

        if (vdim_new > vdim_old)
        {
            // number of vectors is increasing, extend the vector pointers
            int64_t anz = GB_NNZ (A) ;

            int64_t j ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (j = vdim_old + 1 ; j <= vdim_new ; j++)
            { 
                Ap [j] = anz ;
            }
            // A->nvec_nonempty does not change
        }
        A->nvec = vdim_new ;
    }

    if (vdim_new < vdim_old)
    { 
        // number of vectors is decreasing, need to count the new number of
        // non-empty vectors, unless it is done during pruning, just below.
        A->nvec_nonempty = -1 ;         // compute when needed
    }

    //--------------------------------------------------------------------------
    // resize the length of each vector
    //--------------------------------------------------------------------------

    // if vlen is shrinking, delete entries outside the new matrix
    if (vlen_new < vlen_old)
    { 
        GB_OK (GB_selector (NULL, GB_RESIZE_opcode, NULL, false, A, vlen_new-1,
            NULL, Context)) ;
    }

    //--------------------------------------------------------------------------
    // vlen has been resized
    //--------------------------------------------------------------------------

    A->vlen = vlen_new ;
    ASSERT_MATRIX_OK (A, "A vlen resized", GB0) ;

    //--------------------------------------------------------------------------
    // check for conversion to hypersparse or to non-hypersparse
    //--------------------------------------------------------------------------

    return (GB_to_hyper_conform (A, Context)) ;
}

