//------------------------------------------------------------------------------
// GB_fine_slice: create fine hyperslices of B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// For each thread tid, create Bslice [tid] as a fine hyperslice of B.  The i
// and x arrays are the same as B.  When this function returns, the rest of
// GraphBLAS will view Bslice [tid] as a hyperslice, but with non-shallow
// Bslice [tid]->p and either shallow Bslice [tid]->h (if B is hypersparse) or
// non-shallow Bslice [tid]->h (if B is sparse).

// For each fine hyperslice, Bslice [tid]->p is allocated and created here; it
// is not shallow (unlike the coarse slices computed by GB_slice).

// Bslice [tid]->i and Bslice [tid]->x are offset pointers into B, so that
// Bslice [tid]->p [0] == 0 for all slices tid.

// if B is hypersparse, then Bslice [tid]->h is a shallow pointer into B->h,
// where Bslice [tid]->h [0] is the same as B->h [k] if the kth vector of B is
// the first vector of Bslice [tid].

// The matrix dimensions of each slice are the same as B.  All slices have
// vector length B->vlen and vector dimension B->vdim.   The slices are subsets
// of the entries of B, as defined by the Slice array.  The Bslice [tid]
// consists of the entries Slice [tid] to Slice [tid+1]-1 of B.

// This function does O(nthreads+B->nvec) work and allocates up to
// O(nthreads+B->nvec) space, so it could be parallel, but it will tend to be
// used when B->nvec is small (even 1, for GrB_mxv and GrB_vxm).  So it does
// not need to be parallel.

#include "GB_mxm.h"

GrB_Info GB_fine_slice  // slice B into nthreads fine hyperslices
(
    GrB_Matrix B,       // matrix to slice
    int nthreads,       // # of slices to create
    int64_t *Slice,     // array of size nthreads+1 that defines the slice
    GrB_Matrix *Bslice, // array of output slices, of size nthreads
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (B, "B to slice", GB0)) ;
    ASSERT (nthreads > 1) ;
    ASSERT (Bslice != NULL) ;
    ASSERT (Slice != NULL) ;
    ASSERT (Slice [0] == 0) ;
    ASSERT (Slice [nthreads] == GB_NNZ (B)) ;
    for (int tid = 0 ; tid < nthreads ; tid++)
    {
        ASSERT (Slice [tid] <= Slice [tid+1]) ;
    }

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // create the hyperslices
    //--------------------------------------------------------------------------

    for (int tid = 0 ; tid < nthreads ; tid++)
    {

        // Bslice [tid] will contain entries pfirst:plast-1 of B.
        int64_t pfirst = Slice [tid] ;
        int64_t plast  = Slice [tid+1] - 1 ;
        int64_t bslice_nz = plast - pfirst + 1 ;
        int64_t bvec_first = 0 ;
        int64_t bslice_nvec = 0 ;

        if (bslice_nz > 0)
        {

            // find the first column of Bslice [tid]: the column that contains
            // the entry at Bi [pfirst] and Bx [pfirst]
            int64_t pright = B->nvec ;
            bool found ;
            GB_BINARY_SPLIT_SEARCH (pfirst, B->p, bvec_first, pright, found) ;
            if (!found)
            { 
                bvec_first-- ;
            }
            ASSERT (B->p [bvec_first] <= pfirst) ;
            ASSERT (pfirst <= B->p [bvec_first+1]) ;

            // find the last column of Bslice [tid]: the column that contains
            // the entry at Bi [plast] and Bx [plast]
            int64_t bvec_last = bvec_first ;
            pright = B->nvec ;
            GB_BINARY_SPLIT_SEARCH (plast, B->p, bvec_last, pright, found) ;
            if (!found)
            { 
                bvec_last-- ;
            }
            ASSERT (B->p [bvec_last] <= plast && plast < B->p [bvec_last+1]) ;

            // total number of vectors in B
            bslice_nvec = bvec_last - bvec_first + 1 ;

        }

        // allocate Bslice [tid].  Bslice [tid]->p is always allocated.  Bslice
        // [tid] will always eventually be hypersparse.  However,
        // Bslice[tid]->h will be a shallow offset into B->h if B is
        // hypersparse, so GB_new should not allocate h (initially creating a
        // non-hypersparse Bslice [tid]).  If B is not hypersparse, then
        // Bslice[tid]->h must be allocated.  As a result, GB_new should create
        // Bslice [tid] as initially hypersparse if B is not hypersparse.
        // Thus, in both cases, GB_new constructs Bslice [tid] with the
        // opposite hypersparsity status of B.

        Bslice [tid] = NULL ;
        GB_NEW (&(Bslice [tid]), B->type, B->vlen, B->vdim, GB_Ap_malloc,
            B->is_csc, GB_SAME_HYPER_AS (!(B->is_hyper)), GB_ALWAYS_HYPER,
            bslice_nvec, NULL) ;
        if (info != GrB_SUCCESS)
        {
            // out of memory
            for (int i = 0 ; i < tid ; i++)
            { 
                GB_MATRIX_FREE (&(Bslice [i])) ;
            }
            return (info) ;
        }

        // Bslice [tid] is always a hyperslice
        (Bslice [tid])->is_hyper = true ;
        (Bslice [tid])->is_slice = true ;
        (Bslice [tid])->hfirst = 0 ;      // unused
        (Bslice [tid])->plen = bslice_nvec ;
        (Bslice [tid])->nvec = bslice_nvec ;

        // Bslice has shallow pointers into B->i and B->x
        (Bslice [tid])->i = B->i + pfirst ;
        (Bslice [tid])->i_shallow = true ;
        GB_void *restrict Bx = B->x ;
        (Bslice [tid])->x = Bx + pfirst * B->type->size ;
        (Bslice [tid])->x_shallow = true ;

        // Bslice->h hyperlist
        if (B->is_hyper)
        { 
            // B is hypersparse; the columns of Bslice [tid] are B->h
            // [bvec_first:bvec_last].  Bslice [tid] is a hyperslice (with an
            // explict h list, as a shallow pointer into B->h).
            ASSERT ((Bslice [tid])->h == NULL) ;
            (Bslice [tid])->h = B->h + bvec_first ;
            (Bslice [tid])->h_shallow = true ;
        }
        else
        { 
            // the columns of Bslice [tid] are [bvec_first:bvec_last].
            // Bslice [tid] is a hyperslice (with an explicit h list)
            ASSERT ((Bslice [tid])->h != NULL) ;
            ASSERT ((Bslice [tid])->h_shallow == false) ;
            for (int64_t k = 0 ; k < bslice_nvec ; k++)
            {
                (Bslice [tid])->h [k] = bvec_first + k ;
            }
        }

        // Bslice->p is always allocated fresh by GB_new.
        ASSERT ((Bslice [tid])->p != NULL) ;
        ASSERT ((Bslice [tid])->p_shallow == false) ;
        (Bslice [tid])->p [0] = 0 ;
        for (int64_t k = 1 ; k < bslice_nvec ; k++)
        { 
            // construct Bslice [tid]->p
            (Bslice [tid])->p [k] = B->p [bvec_first + k] - pfirst ;
        }
        (Bslice [tid])->p [bslice_nvec] = bslice_nz ;
        (Bslice [tid])->nvec_nonempty = -1 ;

        (Bslice [tid])->nzmax = bslice_nz ;
        (Bslice [tid])->magic = GB_MAGIC ;

        ASSERT_OK (GB_check (Bslice [tid], "Bslice", GB0)) ;
    }

    //--------------------------------------------------------------------------
    // return the slices
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

