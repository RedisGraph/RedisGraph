//------------------------------------------------------------------------------
// GB_slice: create hypersparse shallow slices of a matrix B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// For each slice s, create Bslice [s] as a slice or hyperslice of B.  The i
// and x arrays are the same as B.

// The p array is an offset into Bp (that is, Bp + Slice [s]), which means that
// p [0] will not be zero (except for Bslice [0]).  If B is hypersparse, the h
// array is also an offset into B->h.  If B is standard, then Bslice [s]
// becomes an implicit hypersparse matrix.  Its h array is NULL, and the h list
// is implicit: h[0..nvec-1] is implicitly [hfirst, hfirst+1, ...
// hfirst+nvec-1], where nvec = Slice [s+1] - Slice [s].

// The matrix dimensions of each slice are the same as B.  All slices have
// vector length B->vlen and vector dimension B->vdim.   The slices are subsets
// of the vectors of B, as defined by the Slice array.  The Bslice [s] consists
// of the vectors Slice [s] to Slice [s+1]-1.

// This function does only O(nslices) work and allocates O(nslices) space, so
// it does not need to be parallel.

#include "GB.h"

GrB_Info GB_slice       // slice B into nslices slices or hyperslices
(
    GrB_Matrix B,       // matrix to slice
    int nslices,        // # of slices to create
    int64_t *Slice,     // array of size nslices+1 that defines the slice
    GrB_Matrix *Bslice, // array of output slices, of size nslices
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (B, "B to slice", GB0)) ;
    ASSERT (nslices >= 1) ;
    ASSERT (Bslice != NULL) ;
    ASSERT (Slice != NULL) ;
    ASSERT (Slice [0] == 0) ;
    ASSERT (Slice [nslices] == B->nvec) ;
    for (int s = 0 ; s < nslices ; s++)
    {
        ASSERT (Slice [s] <= Slice [s+1]) ;
    }

    GrB_Info info ;

    // quick return
    if (nslices == 1)
    { 
        // the caller must not free Bslice [0]
        Bslice [0] = B ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // create the slices or hyperslices
    //--------------------------------------------------------------------------

    for (int s = 0 ; s < nslices ; s++)
    {
        // printf ("\n================== slice %d\n", s) ;

        // Bslice [s] = B (:, bcol_first:bcol_last)
        int64_t bvec_first  = Slice [s] ;
        int64_t bvec_last   = Slice [s+1] - 1 ;
        int64_t bslice_nvec = bvec_last - bvec_first + 1 ;

        // printf ("first "GBd" last "GBd" nvec "GBd"\n", 
        // bvec_first, bvec_last, bslice_nvec) ;

        // allocate just the header for Bslice [s]; all content is shallow
        Bslice [s] = NULL ;
        GB_NEW (&(Bslice [s]), B->type, B->vlen, B->vdim, GB_Ap_null,
            B->is_csc, GB_SAME_HYPER_AS (B->is_hyper), B->hyper_ratio,
            bslice_nvec, NULL) ;
        if (info != GrB_SUCCESS)
        {
            // out of memory
            for (int i = 0 ; i < s ; i++)
            { 
                GB_MATRIX_FREE (&(Bslice [i])) ;
            }
            return (GB_OUT_OF_MEMORY) ;
        }

        // Bslice [s] is a slice or hyperslice
        (Bslice [s])->is_slice = true ;

        // Bslice has shallow pointers into B->i and B->x
        (Bslice [s])->i = B->i ; (Bslice [s])->i_shallow = true ;
        (Bslice [s])->x = B->x ; (Bslice [s])->x_shallow = true ;
        (Bslice [s])->h_shallow = true ;

        // Bslice->h hyperlist
        if (B->is_hyper)
        { 
            // the columns of Bslice [s] are B->h [bvec_first:bvec_last].
            // Bslice [s] is a hyperslice (with an explict h list).
            (Bslice [s])->h = B->h + bvec_first ;
            (Bslice [s])->hfirst = 0 ;      // unused
        }
        else
        { 
            // the columns of Bslice [s] are [bvec_first:bvec_last].
            // Bslice [s] is a slice (with an implicit h list)
            (Bslice [s])->h = NULL ;
            (Bslice [s])->hfirst = bvec_first ;
        }

        // Bslice->p pointers
        (Bslice [s])->p = B->p + bvec_first ;
        (Bslice [s])->p_shallow = true ;
        (Bslice [s])->plen = bslice_nvec ;

        (Bslice [s])->nvec = bslice_nvec ;

        if (B->nvec_nonempty == B->nvec)
        { 
            // all vectors present in B, so all vectors present in the slice
            (Bslice [s])->nvec_nonempty = bslice_nvec ;
        }
        else
        { 
            (Bslice [s])->nvec_nonempty = -1 ;
        }

        (Bslice [s])->nzmax = B->nzmax ;
        (Bslice [s])->magic = GB_MAGIC ;

        ASSERT_OK (GB_check (Bslice [s], "Bslice", GB0)) ;
    }

    //--------------------------------------------------------------------------
    // return the slices
    //--------------------------------------------------------------------------

    return (GrB_SUCCESS) ;
}

