//------------------------------------------------------------------------------
// GB_bitmap_AxB_saxpy_template.c: C<#M>+=A*B when C is bitmap
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_AxB_saxpy_sparsity determines the sparsity structure for C<M or !M>=A*B
// or C=A*B, and this template is used when C is bitmap.  C can be modified
// in-place if the accum operator is the same as the monoid.

#undef  GB_FREE_ALL
#define GB_FREE_ALL                         \
{                                           \
    GB_FREE_WORK (&Wf, Wf_size) ;           \
    GB_FREE_WORK (&Wcx, Wcx_size) ;         \
    GB_WERK_POP (H_slice, int64_t) ;        \
    GB_WERK_POP (A_slice, int64_t) ;        \
    GB_WERK_POP (B_slice, int64_t) ;        \
    GB_WERK_POP (M_ek_slicing, int64_t) ;   \
}

#undef  GB_C_IS_BITMAP
#define GB_C_IS_BITMAP 1

{

    //--------------------------------------------------------------------------
    // declare workspace
    //--------------------------------------------------------------------------

    int8_t  *restrict Wf  = NULL ; size_t Wf_size = 0 ;
    GB_void *restrict Wcx = NULL ; size_t Wcx_size = 0 ;
    GB_WERK_DECLARE (H_slice, int64_t) ;
    GB_WERK_DECLARE (A_slice, int64_t) ;
    GB_WERK_DECLARE (B_slice, int64_t) ;
    GB_WERK_DECLARE (M_ek_slicing, int64_t) ;

    //--------------------------------------------------------------------------
    // determine max # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // get C, M, A, and B
    //--------------------------------------------------------------------------

    ASSERT (GB_IS_BITMAP (C)) ;                 // C is always bitmap
    int8_t *restrict Cb = C->b ;
    const int64_t cvlen = C->vlen ;
    ASSERT (C->vlen == A->vlen) ;
    ASSERT (C->vdim == B->vdim) ;
    ASSERT (A->vdim == B->vlen) ;
    int64_t cnvals = C->nvals ;

    const int8_t *restrict Bb = B->b ;
    const bool B_iso = B->iso ;
    const int64_t bvlen = B->vlen ;
    const int64_t bvdim = B->vdim ;

    const bool B_jumbled = B->jumbled ;
    const int64_t bnz = GB_nnz_held (B) ;

    const bool B_is_bitmap = GB_IS_BITMAP (B) ;
    ASSERT (!GB_IS_SPARSE (B)) ;
    ASSERT (!GB_IS_HYPERSPARSE (B)) ;

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ah = A->h ;
    const int8_t  *restrict Ab = A->b ;
    const int64_t *restrict Ai = A->i ;
    const bool A_iso = A->iso ;
    const int64_t anvec = A->nvec ;
    const int64_t avlen = A->vlen ;
    const int64_t avdim = A->vdim ;

    const bool A_jumbled = A->jumbled ;
    const int64_t anz = GB_nnz_held (A) ;

    const bool A_is_sparse = GB_IS_SPARSE (A) ;
    const bool A_is_hyper = GB_IS_HYPERSPARSE (A) ;
    const bool A_is_bitmap = GB_IS_BITMAP (A) ;
    const bool A_is_sparse_or_hyper = A_is_sparse || A_is_hyper ;

    const int64_t *restrict Mp = NULL ;
    const int64_t *restrict Mh = NULL ;
    const int8_t  *restrict Mb = NULL ;
    const int64_t *restrict Mi = NULL ;
    const GB_void *restrict Mx = NULL ;
    size_t msize = 0 ;
    int64_t mnvec = 0 ;
    int64_t mvlen = 0 ;
    const bool M_is_hyper  = GB_IS_HYPERSPARSE (M) ;
    const bool M_is_sparse = GB_IS_SPARSE (M) ;
    const bool M_is_sparse_or_hyper = M_is_hyper || M_is_sparse ;
    const bool M_is_bitmap = GB_IS_BITMAP (M) ;
    const bool M_is_full   = GB_IS_FULL (M) ;
    int M_nthreads = 0 ;
    int M_ntasks = 0 ;

    if (M != NULL)
    {
        ASSERT (C->vlen == M->vlen) ;
        ASSERT (C->vdim == M->vdim) ;
        Mp = M->p ;
        Mh = M->h ;
        Mb = M->b ;
        Mi = M->i ;
        Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;
        msize = M->type->size ;
        mnvec = M->nvec ;
        mvlen = M->vlen ;

        GB_SLICE_MATRIX (M, 8, chunk) ;

        // if M is sparse or hypersparse, scatter it into the C bitmap
        if (M_is_sparse_or_hyper)
        { 
            // Cb [pC] += 2 for each entry M(i,j) in the mask
            GB_bitmap_M_scatter (C,
                NULL, 0, GB_ALL, NULL, NULL, 0, GB_ALL, NULL,
                M, Mask_struct, GB_ASSIGN, GB_BITMAP_M_SCATTER_PLUS_2,
                M_ek_slicing, M_ntasks, M_nthreads, Context) ;
            // the bitmap of C now contains:
            //  Cb (i,j) = 0:   cij not present, mij zero
            //  Cb (i,j) = 1:   cij present, mij zero
            //  Cb (i,j) = 2:   cij not present, mij 1
            //  Cb (i,j) = 3:   cij present, mij 1
        }
    }

    #if !GB_A_IS_PATTERN
    const GB_ATYPE *restrict Ax = (GB_ATYPE *) A->x ;
    #endif
    #if !GB_B_IS_PATTERN
    const GB_BTYPE *restrict Bx = (GB_BTYPE *) B->x ;
    #endif
    #if !GB_IS_ANY_PAIR_SEMIRING
          GB_CTYPE *restrict Cx = (GB_CTYPE *) C->x ;
    #endif

    //--------------------------------------------------------------------------
    // select the method
    //--------------------------------------------------------------------------

    if (A_is_sparse_or_hyper)
    {

        //-----------------------------------------------------
        // C                =               A     *     B
        //-----------------------------------------------------

        // bitmap           .               hyper       bitmap
        // bitmap           .               sparse      bitmap
        // bitmap           .               hyper       full 
        // bitmap           .               sparse      full

        //-----------------------------------------------------
        // C               <M>=             A     *     B
        //-----------------------------------------------------

        // bitmap           any             hyper       bitmap
        // bitmap           any             sparse      bitmap
        // bitmap           bitmap/full     hyper       full
        // bitmap           bitmap/full     sparse      full

        //-----------------------------------------------------
        // C               <!M>=            A     *     B
        //-----------------------------------------------------

        // bitmap           any             hyper       bitmap
        // bitmap           any             sparse      bitmap
        // bitmap           any             hyper       full 
        // bitmap           any             sparse      full

        // construct the tasks
        ASSERT (GB_IS_BITMAP (B) || GB_IS_FULL (B)) ;
        int nthreads, ntasks, nfine_tasks_per_vector ;
        bool use_coarse_tasks, use_atomics ;
        GB_AxB_saxpy4_tasks (&ntasks, &nthreads, &nfine_tasks_per_vector,
            &use_coarse_tasks, &use_atomics, anz, bnz, bvdim, cvlen, Context) ;
        if (!use_coarse_tasks)
        {
            // slice the matrix A for each team of fine tasks
            GB_WERK_PUSH (A_slice, nfine_tasks_per_vector + 1, int64_t) ;
            if (A_slice == NULL)
            { 
                // out of memory
                GB_FREE_ALL ;
                return (GrB_OUT_OF_MEMORY) ;
            }
            GB_pslice (A_slice, Ap, anvec, nfine_tasks_per_vector, true) ;
        }

        if (M == NULL)
        {

            //------------------------------------------------------------------
            // C = A*B, no mask, A sparse/hyper, B bitmap/full
            //------------------------------------------------------------------

            #define GB_NO_MASK 1
            #define GB_MASK_IS_SPARSE_OR_HYPER 0
            #define GB_MASK_IS_BITMAP_OR_FULL  0
            #undef  keep
            #define keep 1
            if (B_is_bitmap)
            { 
                // A is sparse/hyper, B is bitmap, no mask
                #undef  GB_B_IS_BITMAP
                #define GB_B_IS_BITMAP 1
                #include "GB_bitmap_AxB_saxpy_A_sparse_B_bitmap_template.c"
            }
            else
            { 
                // A is sparse/hyper, B is full, no mask
                #undef  GB_B_IS_BITMAP
                #define GB_B_IS_BITMAP 0
                #include "GB_bitmap_AxB_saxpy_A_sparse_B_bitmap_template.c"
            }
            #undef GB_MASK_IS_SPARSE_OR_HYPER
            #undef GB_MASK_IS_BITMAP_OR_FULL
            #undef GB_NO_MASK

        }
        else if (M_is_sparse_or_hyper)
        {

            //------------------------------------------------------------------
            // C<M> or <!M> = A*B, M and A are sparse/hyper, B bitmap/full
            //------------------------------------------------------------------

            #define GB_NO_MASK 0
            #define GB_MASK_IS_SPARSE_OR_HYPER 1
            #define GB_MASK_IS_BITMAP_OR_FULL  0
            #undef  keep
            const int8_t keep = (Mask_comp) ? 1 : 3 ;
            if (B_is_bitmap)
            { 
                // A is sparse/hyper, B is bitmap, M is sparse/hyper
                #undef  GB_B_IS_BITMAP
                #define GB_B_IS_BITMAP 1
                #include "GB_bitmap_AxB_saxpy_A_sparse_B_bitmap_template.c"
            }
            else
            { 
                // A is sparse/hyper, B is full, M is sparse/hyper
                #undef  GB_B_IS_BITMAP
                #define GB_B_IS_BITMAP 0
                #include "GB_bitmap_AxB_saxpy_A_sparse_B_bitmap_template.c"
            }
            #undef GB_MASK_IS_SPARSE_OR_HYPER
            #undef GB_MASK_IS_BITMAP_OR_FULL

        }
        else
        {

            //------------------------------------------------------------------
            // C<M> or <!M> = A*B, M bitmap, A sparse, B bitmap
            //------------------------------------------------------------------

            #define GB_MASK_IS_SPARSE_OR_HYPER 0
            #define GB_MASK_IS_BITMAP_OR_FULL  1
            #undef  keep
            #define keep 1
            if (B_is_bitmap)
            { 
                // A is sparse/hyper, B is bitmap, M is bitmap/full
                #undef  GB_B_IS_BITMAP
                #define GB_B_IS_BITMAP 1
                #include "GB_bitmap_AxB_saxpy_A_sparse_B_bitmap_template.c"
            }
            else
            { 
                // A is sparse/hyper, B is full, M is bitmap/full
                #undef  GB_B_IS_BITMAP
                #define GB_B_IS_BITMAP 0
                #include "GB_bitmap_AxB_saxpy_A_sparse_B_bitmap_template.c"
            }
            #undef GB_MASK_IS_SPARSE_OR_HYPER
            #undef GB_MASK_IS_BITMAP_OR_FULL
            #undef GB_NO_MASK
        }

        #undef GB_B_IS_BITMAP

    }
    else
    {

        //-----------------------------------------------------
        // C                =               A     *     B
        //-----------------------------------------------------

        // bitmap           .               bitmap      bitmap
        // bitmap           .               full        bitmap
        // bitmap           .               bitmap      full
        // full             .               full        full

        //-----------------------------------------------------
        // C               <M>=             A     *     B
        //-----------------------------------------------------

        // bitmap           any             bitmap      bitmap
        // bitmap           any             full        bitmap
        // bitmap           bitmap/full     bitmap      full
        // bitmap           bitmap/full     full        full

        //-----------------------------------------------------
        // C               <!M>=            A     *     B
        //-----------------------------------------------------

        // bitmap           any             bitmap      bitmap
        // bitmap           any             full        bitmap
        // bitmap           any             bitmap      full
        // bitmap           any             full        full

        #define GB_TILE_SIZE 64
        #define GB_KTILE_SIZE 8

        double work = ((double) avlen) * ((double) bvlen) * ((double) bvdim) ;
        int nthreads = GB_nthreads (work, chunk, nthreads_max) ;
        int64_t nI_tasks = (bvdim == 0) ? 1 : (1 + (bvdim-1) / GB_TILE_SIZE) ;
        int64_t nJ_tasks = (avlen == 0) ? 1 : (1 + (avlen-1) / GB_TILE_SIZE) ;
        int64_t ntasks = nI_tasks * nJ_tasks ;

        if (M == NULL)
        { 

            //------------------------------------------------------------------
            // C = A*B, no mask, A and B bitmap/full
            //------------------------------------------------------------------

            #define GB_MASK_IS_SPARSE_OR_HYPER 0
            #define GB_MASK_IS_BITMAP_OR_FULL  0
            #undef  keep
            #define keep 1
            #include "GB_bitmap_AxB_saxpy_A_bitmap_B_bitmap_template.c"
            #undef GB_MASK_IS_SPARSE_OR_HYPER
            #undef GB_MASK_IS_BITMAP_OR_FULL

        }
        else if (M_is_sparse_or_hyper)
        { 

            //------------------------------------------------------------------
            // C<M> or <!M> = A*B, M sparse/hyper, A and B bitmap/full
            //------------------------------------------------------------------

            #define GB_MASK_IS_SPARSE_OR_HYPER 1
            #define GB_MASK_IS_BITMAP_OR_FULL  0
            #undef  keep
            const int8_t keep = (Mask_comp) ? 1 : 3 ;
            #include "GB_bitmap_AxB_saxpy_A_bitmap_B_bitmap_template.c"
            #undef GB_MASK_IS_SPARSE_OR_HYPER
            #undef GB_MASK_IS_BITMAP_OR_FULL

        }
        else
        { 

            //------------------------------------------------------------------
            // C<M> or <!M> = A*B, all matrices bitmap/full
            //------------------------------------------------------------------

            #define GB_MASK_IS_SPARSE_OR_HYPER 0
            #define GB_MASK_IS_BITMAP_OR_FULL  1
            #undef  keep
            #define keep 1
            #include "GB_bitmap_AxB_saxpy_A_bitmap_B_bitmap_template.c"
            #undef GB_MASK_IS_SPARSE_OR_HYPER
            #undef GB_MASK_IS_BITMAP_OR_FULL
        }
    }

    C->nvals = cnvals ;

    //--------------------------------------------------------------------------
    // if M is sparse, clear it from the C bitmap
    //--------------------------------------------------------------------------

    if (M_is_sparse_or_hyper)
    { 
        // Cb [pC] -= 2 for each entry M(i,j) in the mask
        GB_bitmap_M_scatter (C,
            NULL, 0, GB_ALL, NULL, NULL, 0, GB_ALL, NULL,
            M, Mask_struct, GB_ASSIGN, GB_BITMAP_M_SCATTER_MINUS_2,
            M_ek_slicing, M_ntasks, M_nthreads, Context) ;
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_FREE_ALL ;
}

#undef GB_FREE_ALL
#undef GB_C_IS_BITMAP

