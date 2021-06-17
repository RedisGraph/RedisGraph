//------------------------------------------------------------------------------
// GB_convert_bitmap_to_sparse: convert a matrix from bitmap to sparse
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

#define GB_FREE_ALL     \
{                       \
    GB_FREE (Ap) ;      \
    GB_FREE (Ai) ;      \
    GB_FREE (Ax) ;      \
}

GrB_Info GB_convert_bitmap_to_sparse    // convert matrix from bitmap to sparse
(
    GrB_Matrix A,               // matrix to convert from bitmap to sparse
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (A, "A converting bitmap to sparse", GB0) ;
    ASSERT (!GB_IS_FULL (A)) ;
    ASSERT (GB_IS_BITMAP (A)) ;
    ASSERT (!GB_IS_SPARSE (A)) ;
    ASSERT (!GB_IS_HYPERSPARSE (A)) ;
    ASSERT (!GB_PENDING (A)) ;      // bitmap never has pending tuples
    ASSERT (!GB_JUMBLED (A)) ;      // bitmap is never jumbled
    ASSERT (!GB_ZOMBIES (A)) ;      // bitmap never has zomies
    GBURBLE ("(bitmap to sparse) ") ;

    //--------------------------------------------------------------------------
    // allocate Ap, Ai, and Ax
    //--------------------------------------------------------------------------

    const int64_t anz = GB_NNZ (A) ;
    const int64_t anzmax = GB_IMAX (anz, 1) ;
    int64_t anvec_nonempty ;
    const int64_t avdim = A->vdim ;
    const size_t asize = A->type->size ;
    int64_t *GB_RESTRICT Ap = GB_MALLOC (avdim+1, int64_t) ; 
    int64_t *GB_RESTRICT Ai = GB_MALLOC (anzmax, int64_t) ;
    GB_void *GB_RESTRICT Ax = GB_MALLOC (anzmax * asize, GB_void) ;
    if (Ap == NULL || Ai == NULL || Ax == NULL)
    { 
        // out of memory
        GB_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // convert to sparse format (Ap, Ai, and Ax)
    //--------------------------------------------------------------------------

    GB_OK (GB_convert_bitmap_worker (Ap, Ai, NULL, Ax, &anvec_nonempty, A,
        Context)) ;

    //--------------------------------------------------------------------------
    // free prior content of A and transplant the new content
    //--------------------------------------------------------------------------

    GB_phbix_free (A) ;

    A->p = Ap ;
    A->p_shallow = false ;

    A->i = Ai ;
    A->i_shallow = false ;

    A->x = Ax ;
    A->x_shallow = false ;

    A->nzmax = anzmax ;
    A->nvals = 0 ;              // only used when A is bitmap

    A->plen = avdim ;
    A->nvec = avdim ;
    A->nvec_nonempty = anvec_nonempty ;

    A->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (A, "A converted from to bitmap to sparse", GB0) ;
    ASSERT (GB_IS_SPARSE (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    return (GrB_SUCCESS) ;
}

