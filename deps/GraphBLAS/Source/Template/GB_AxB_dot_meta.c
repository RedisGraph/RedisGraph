//------------------------------------------------------------------------------
// GB_AxB_dot_meta: C=A'*B or C<M>=A'*B via dot productes
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

// This file is #include'd into GB_AxB_dot.c for the generic case, and in the
// hard-coded semirings, Generated/GB_AxB__*_*_.c.  It constructs the
// dot-product variant of sparse matrix multiplication, C=A'*B, without
// transposing A.

// This method can optionally allocate size(B->vlen) workspace to scatter each
// vector B(:,j), one at a time.  In the default method, this is only done if
// the size of the workspace does not dominate the computation, in case A and B
// are hypersparse.

#define GB_DOT_FREE_WORK                                    \
{                                                           \
    GB_FREE_MEMORY (Flag, bvlen, sizeof (int8_t)) ;         \
    GB_FREE_MEMORY (Work, bvlen, bkj_size) ;                \
}

{

    const GB_atype *restrict Ax = A->x ;
    const GB_btype *restrict Bx = B->x ;

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_NOT_ALIASED_3 (C, M, A, B)) ;
    ASSERT (C->vdim == B->vdim) ;
    ASSERT (C->vlen == A->vdim) ;
    ASSERT (A->vlen == B->vlen) ;

    //--------------------------------------------------------------------------
    // get A and B
    //--------------------------------------------------------------------------

    const int64_t *restrict Ah = A->h ;
    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    bool A_is_hyper = A->is_hyper ;
    int64_t anvec = A->nvec ;

    const int64_t *restrict Bi = B->i ;
    int64_t bvlen = B->vlen ;

    //--------------------------------------------------------------------------
    // determine if B can be scattered
    //--------------------------------------------------------------------------

    // FUTURE: give user control over the decision to use O(bvlen) workspace

    // allow B to be scattered into workspace of size bvlen if bvlen is not too
    // large

    bool B_can_scatter = (bvlen < GB_NNZ (A) + GB_NNZ (B)) ;
    int8_t *restrict Flag = NULL ;
    GB_DOT_WORK_TYPE *restrict Work = NULL ;

    //--------------------------------------------------------------------------
    // start the construction of C
    //--------------------------------------------------------------------------

    int64_t *restrict Ci = C->i ;

    int64_t jlast, cnz, cnz_last ;
    GB_jstartup (C, &jlast, &cnz, &cnz_last) ;

    //--------------------------------------------------------------------------
    // C=A'*B or C<M>=A'*B via dot products
    //--------------------------------------------------------------------------

    if (M != NULL)
    { 
        // C<M> = A'*B via dot products
        #include "GB_AxB_dot_mask.c"
    }
    else
    { 
        // C = A'*B via dot products
        #include "GB_AxB_dot_nomask.c"
    }

    GB_DOT_FREE_WORK ;
}

#undef GB_DOT_FREE_WORK
