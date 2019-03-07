//------------------------------------------------------------------------------
// GB_AxB_dot_meta: C=A'*B or C<M>=A'*B via dot productes
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd into GB_AxB_dot.c for the generic case, and in the
// hard-coded semirings, Generated/GB_AxB__*_*_.c.  It constructs the
// dot-product variant of sparse matrix multiplication, C=A'*B, without
// transposing A.

// parallel: not here.

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
    int64_t anvec = A->nvec ;
    bool A_is_hyper = GB_IS_HYPER (A) ;

    const int64_t *restrict Bi = B->i ;
    int64_t bvlen = B->vlen ;

    //--------------------------------------------------------------------------
    // start the construction of C
    //--------------------------------------------------------------------------

    int64_t *restrict Ci = C->i ;

    int64_t jlast, cnz, cnz_last ;
    GB_jstartup (C, &jlast, &cnz, &cnz_last) ;

    //--------------------------------------------------------------------------
    // C=A'*B, C<M>=A'*B, or C<!M>=A'*B via dot products
    //--------------------------------------------------------------------------

    if (M == NULL)
    { 

        // C = A'*B via dot products
        #include "GB_AxB_dot_nomask.c"

    }
    else
    {

        //----------------------------------------------------------------------
        // get M
        //----------------------------------------------------------------------

        ASSERT (C->vdim == M->vdim) ;
        ASSERT (C->vlen == M->vlen) ;

        const int64_t *restrict Mp = M->p ;
        const int64_t *restrict Mh = M->h ;
        const int64_t *restrict Mi = M->i ;
        const GB_void *restrict Mx = M->x ;
        GB_cast_function cast_M = GB_cast_factory (GB_BOOL_code, M->type->code);
        size_t msize = M->type->size ;
        const int64_t mnvec = M->nvec ;
        int64_t mpleft = 0 ;
        int64_t mpright = mnvec - 1 ;
        bool M_is_hyper = GB_IS_HYPER (M) ;

        if (Mask_comp)
        {
            // C<!M> = A'*B via dot products
            #include "GB_AxB_dot_compmask.c"
        }
        else
        { 
            // C<M> = A'*B via dot products
            #include "GB_AxB_dot_mask.c"
        }
    }
}
