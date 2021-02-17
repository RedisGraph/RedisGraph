//------------------------------------------------------------------------------
// GB_resize: change the size of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_select.h"

#define GB_FREE_ALL         \
{                           \
    GB_FREE (Ax_new) ;      \
    GB_FREE (Ab_new) ;      \
    GB_phbix_free (A) ;     \
}

//------------------------------------------------------------------------------
// GB_resize: resize a GrB_Matrix
//------------------------------------------------------------------------------

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

    GrB_Info info ;
    GB_void *GB_RESTRICT Ax_new = NULL ;
    int8_t  *GB_RESTRICT Ab_new = NULL ;
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

    if (vdim_new == vdim_old && vlen_new == vlen_old)
    { 
        // nothing to do
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    // only do so if either dimension is shrinking, or if pending tuples exist
    // and vdim_old <= 1 and vdim_new > 1, since in that case, Pending->j has
    // not been allocated yet, but would be required in the resized matrix.
    // If A is jumbled, it must be sorted.

    if (vdim_new < vdim_old || vlen_new < vlen_old || A->jumbled ||
        (GB_PENDING (A) && vdim_old <= 1 && vdim_new > 1))
    { 
        GB_MATRIX_WAIT (A) ;
        ASSERT_MATRIX_OK (A, "A to resize, wait", GB0) ;
    }

    ASSERT (!GB_JUMBLED (A)) ;

    //--------------------------------------------------------------------------
    // resize the matrix
    //--------------------------------------------------------------------------

    bool A_is_bitmap = GB_IS_BITMAP (A) ;
    bool A_is_full = GB_IS_FULL (A) ;
    bool A_is_shrinking = (vdim_new <= vdim_old && vlen_new <= vlen_old) ;

    if ((A_is_full || A_is_bitmap) && A_is_shrinking)
    {

        //----------------------------------------------------------------------
        // A is full or bitmap
        //----------------------------------------------------------------------

        // get the old and new dimensions
        int64_t anz_old = vlen_old * vdim_old ;
        int64_t anz_new = vlen_new * vdim_new ;
        size_t nzmax_new = GB_IMAX (anz_new, 1) ;
        size_t nzmax_old = A->nzmax ;
        bool in_place = A_is_full && (vlen_new == vlen_old || vdim_new <= 1) ;
        size_t asize = A->type->size ;

        //----------------------------------------------------------------------
        // allocate or reallocate A->x and A->b
        //----------------------------------------------------------------------

        bool ok = true ;
        if (in_place)
        { 
            // reallocate A->x in-place; no data movement needed
            GB_REALLOC (A->x, nzmax_new*asize, nzmax_old*asize, GB_void, &ok) ;
        }
        else
        { 
            // allocate new space for A->x
            Ax_new = GB_MALLOC (nzmax_new*asize, GB_void) ;
            ok = (Ax_new != NULL) ;
            if (A_is_bitmap)
            {
                // allocate new space for A->b
                Ab_new = GB_MALLOC (nzmax_new*asize, int8_t) ;
                ok = ok && (Ab_new != NULL) ;
            }
        }
        if (!ok)
        { 
            // out of memory
            GB_FREE_ALL ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // move data if not in-place
        //----------------------------------------------------------------------

        if (!in_place)
        {

            //------------------------------------------------------------------
            // determine number of threads to use
            //------------------------------------------------------------------

            GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
            int nthreads = GB_nthreads (anz_new, chunk, nthreads_max) ;

            //------------------------------------------------------------------
            // resize Ax
            //------------------------------------------------------------------
        
            GB_void *GB_RESTRICT Ax_old = A->x ;

            int64_t j ;
            if (vdim_new <= 4*nthreads)
            {
                // use all threads for each vector
                for (j = 0 ; j < vdim_new ; j++)
                { 
                    GB_void *pdest = Ax_new + j * vlen_new * asize ;
                    GB_void *psrc  = Ax_old + j * vlen_old * asize ;
                    GB_memcpy (pdest, psrc, vlen_new * asize, nthreads) ;
                }
            }
            else
            {
                // use a single thread for each vector
                #pragma omp parallel for num_threads(nthreads) schedule(static)
                for (j = 0 ; j < vdim_new ; j++)
                { 
                    GB_void *pdest = Ax_new + j * vlen_new * asize ;
                    GB_void *psrc  = Ax_old + j * vlen_old * asize ;
                    memcpy (pdest, psrc, vlen_new * asize) ;
                }
            }
            A->x = Ax_new ;
            GB_FREE (Ax_old) ;

            //------------------------------------------------------------------
            // resize Ab if A is bitmap, and count the # of entries
            //------------------------------------------------------------------

            if (A_is_bitmap)
            { 
                int8_t *GB_RESTRICT Ab_old = A->b ;
                int64_t pnew ;
                int64_t anvals = 0 ;
                #pragma omp parallel for num_threads(nthreads) \
                    schedule(static) reduction(+:anvals)
                for (pnew = 0 ; pnew < anz_new ; pnew++)
                { 
                    int64_t i = pnew % vlen_new ;
                    int64_t j = pnew / vlen_new ;
                    int64_t pold = i + j * vlen_old ;
                    int8_t ab = Ab_old [pold] ;
                    Ab_new [pnew] = ab ;
                    anvals += ab ;
                }
                A->nvals = anvals ;
                A->b = Ab_new ;
                GB_FREE (Ab_old) ;
            }
        }

        //----------------------------------------------------------------------
        // adjust dimensions and return result
        //----------------------------------------------------------------------

        A->vdim = vdim_new ;
        A->vlen = vlen_new ;
        A->nzmax = nzmax_new ;
        A->nvec = vdim_new ;
        A->nvec_nonempty = (vlen_new == 0) ? 0 : vdim_new ;
        ASSERT_MATRIX_OK (A, "A bitmap/full shrunk", GB0) ;
        return (GrB_SUCCESS) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // convert A to hypersparse and resize it
        //----------------------------------------------------------------------

        // convert to hypersparse
        GB_OK (GB_convert_any_to_hyper (A, Context)) ;
        ASSERT (GB_IS_HYPERSPARSE (A)) ;

        // resize the number of sparse vectors
        int64_t *GB_RESTRICT Ah = A->h ;
        int64_t *GB_RESTRICT Ap = A->p ;
        A->vdim = vdim_new ;

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

        if (vdim_new < vdim_old)
        { 
            // number of vectors is decreasing, need to count the new number of
            // non-empty vectors: done during pruning or by selector, below.
            A->nvec_nonempty = -1 ;     // recomputed just below
        }

        //----------------------------------------------------------------------
        // resize the length of each vector
        //----------------------------------------------------------------------

        // if vlen is shrinking, delete entries outside the new matrix
        if (vlen_new < vlen_old)
        { 
            GB_OK (GB_selector (NULL /* A in-place */, GB_RESIZE_opcode, NULL,
                false, A, vlen_new-1, NULL, Context)) ;
        }

        //----------------------------------------------------------------------
        // vlen has been resized
        //----------------------------------------------------------------------

        A->vlen = vlen_new ;
        ASSERT_MATRIX_OK (A, "A vlen resized", GB0) ;

        //----------------------------------------------------------------------
        // conform the matrix to its desired sparsity structure
        //----------------------------------------------------------------------

        return (GB_conform (A, Context)) ;
    }
}

