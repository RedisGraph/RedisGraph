//------------------------------------------------------------------------------
// GB_emult_template:  phase1 and phase2 for C=A.*B, C<M>=A.*B, C<!M>=A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Computes C=A.*B, C<M>=A.*B, or C<!M>=A.*B. 

// C is sparse, hypersparse, or bitmap.  M, A, and B can have any sparsity
// structure.  If both A and B are full, then GB_add is used instead (this is
// the only case where C can be full).

// phase1: does not compute C itself, but just counts the # of entries in each
// vector of C.  Fine tasks compute the # of entries in their slice of a
// single vector of C, and the results are cumsum'd.

// phase2: computes C, using the counts computed by phase1.

{

    // iB_first is unused if the operator is FIRST or PAIR
    #include "GB_unused.h"

    //--------------------------------------------------------------------------
    // get A, B, M, and C
    //--------------------------------------------------------------------------

    int taskid ;

    const int64_t *GB_RESTRICT Ap = A->p ;
    const int64_t *GB_RESTRICT Ah = A->h ;
    const int8_t  *GB_RESTRICT Ab = A->b ;
    const int64_t *GB_RESTRICT Ai = A->i ;
    const int64_t vlen = A->vlen ;
    const bool A_is_hyper = GB_IS_HYPERSPARSE (A) ;
    const bool A_is_sparse = GB_IS_SPARSE (A) ;
    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_is_full = GB_as_if_full (A) ;
    int A_nthreads, A_ntasks ;

    const int64_t *GB_RESTRICT Bp = B->p ;
    const int64_t *GB_RESTRICT Bh = B->h ;
    const int8_t  *GB_RESTRICT Bb = B->b ;
    const int64_t *GB_RESTRICT Bi = B->i ;
    const bool B_is_hyper = GB_IS_HYPERSPARSE (B) ;
    const bool B_is_sparse = GB_IS_SPARSE (B) ;
    const bool B_is_bitmap = GB_IS_BITMAP (B) ;
    const bool B_is_full = GB_as_if_full (B) ;
    int B_nthreads, B_ntasks ;

    const int64_t *GB_RESTRICT Mp = NULL ;
    const int64_t *GB_RESTRICT Mh = NULL ;
    const int8_t  *GB_RESTRICT Mb = NULL ;
    const int64_t *GB_RESTRICT Mi = NULL ;
    const GB_void *GB_RESTRICT Mx = NULL ;
    const bool M_is_hyper = GB_IS_HYPERSPARSE (M) ;
    const bool M_is_sparse = GB_IS_SPARSE (M) ;
    const bool M_is_bitmap = GB_IS_BITMAP (M) ;
    const bool M_is_full = GB_as_if_full (M) ;
    const bool M_is_sparse_or_hyper = M_is_sparse || M_is_hyper ;
    int M_nthreads, M_ntasks ;
    size_t msize = 0 ;
    if (M != NULL)
    { 
        Mp = M->p ;
        Mh = M->h ;
        Mb = M->b ;
        Mi = M->i ;
        Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;
        msize = M->type->size ;
    }

    #if defined ( GB_PHASE_2_OF_2 )
    const GB_ATYPE *GB_RESTRICT Ax = (GB_ATYPE *) A->x ;
    const GB_BTYPE *GB_RESTRICT Bx = (GB_BTYPE *) B->x ;
    const int64_t  *GB_RESTRICT Cp = C->p ;
    const int64_t  *GB_RESTRICT Ch = C->h ;
          int8_t   *GB_RESTRICT Cb = C->b ;
          int64_t  *GB_RESTRICT Ci = C->i ;
          GB_CTYPE *GB_RESTRICT Cx = (GB_CTYPE *) C->x ;
    // when C is bitmap or full:
    const int64_t cnz = GB_NNZ_HELD (C) ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    #endif

    //--------------------------------------------------------------------------
    // C=A.*B, C<M>=A.*B, or C<!M>=A.*B: 2 cases for the sparsity of C
    //--------------------------------------------------------------------------

    #if defined ( GB_PHASE_1_OF_2 )

        // phase1: symbolic phase
        // C is sparse or hypersparse (never bitmap or full)
        #include "GB_sparse_emult_template.c"

    #else

        // phase2: numerical phase
        if (C_sparsity == GxB_SPARSE || C_sparsity == GxB_HYPERSPARSE)
        { 
            // C is sparse or hypersparse
            #include "GB_sparse_emult_template.c"
        }
        else // C_sparsity == GxB_BITMAP
        { 
            // C is bitmap (phase2 only)
            ASSERT (C_sparsity == GxB_BITMAP) ;
            #include "GB_bitmap_emult_template.c"
        }

    #endif
}

