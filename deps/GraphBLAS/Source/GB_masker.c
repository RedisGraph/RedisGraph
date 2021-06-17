//------------------------------------------------------------------------------
// GB_masker: R = masker (C, M, Z) constructs R for C<M>=Z
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_masker (R, C, M, Z), does R=C ; R<M>=Z.  No typecasting is performed.
// The operation is similar to both R=C+Z via GB_add and R=C.*Z via GB_emult,
// depending on the value of the mask.

// GB_masker is only called by GB_mask, which itself is only called by
// GB_accum_mask.

// Let R be the result of the mask.  In the caller, R is written back into the
// final C matrix, but in GB_masker, C is a read-only matrix.  Consider the
// following table, where "add" is the result of C+Z, an "emult" is the result
// of C.*Z.

//                                      R = masker (C,M,Z)

// C(i,j)   Z(i,j)  add     emult       M(i,j)=1    M(i,j)=0

// ------   ------  ------  ------      --------    --------

//  cij     zij     cij+zij cij*zij     zij         cij

//   -      zij     zij     -           zij         -

//  cij     -       cij     -           -           cij

//   -      -       -       -           -           -

// As a result, GB_masker is very similar to GB_add and GB_emult.  The
// vectors that appear in R are bounded by the set union of C and Z, just
// like GB_add when the mask is *not* present.  The pattern of R is bounded
// by the pattern of C+Z, also ignoring the mask.

// C is always sparse or hypersparse; if C is bitmap or full, GB_subassign is
// used instead, since C(:,:)<M>=Z can directly modify C in that case, without
// creating zombies or pending tuples, in GB_bitmap_assign.

// M and Z can have any sparsity structure: sparse, hypersparse, bitmap, or
// full.  R is constructed as sparse, hypersparse, or bitmap, depending on
// the sparsity of M and Z, as determined by GB_masker_sparsity.

#include "GB_mask.h"
#include "GB_add.h"
#define GB_FREE_ALL ;

GrB_Info GB_masker          // R = masker (C, M, Z)
(
    GrB_Matrix *Rhandle,    // output matrix (unallocated on input)
    const bool R_is_csc,    // format of output matrix R
    const GrB_Matrix M,     // required input mask
    const bool Mask_comp,   // descriptor for M
    const bool Mask_struct, // if true, use the only structure of M
    const GrB_Matrix C,     // input C matrix
    const GrB_Matrix Z,     // input Z matrix
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    ASSERT (Rhandle != NULL) ;

    ASSERT_MATRIX_OK (M, "M for masker", GB0) ;
    ASSERT (!GB_PENDING (M)) ;
    ASSERT (!GB_JUMBLED (M)) ;
    ASSERT (!GB_ZOMBIES (M)) ;

    ASSERT_MATRIX_OK (C, "C for masker", GB0) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;

    ASSERT_MATRIX_OK (Z, "Z for masker", GB0) ;
    ASSERT (!GB_PENDING (Z)) ;
    ASSERT (!GB_JUMBLED (Z)) ;
    ASSERT (!GB_ZOMBIES (Z)) ;

    ASSERT (!GB_IS_BITMAP (C)) ;    // GB_masker not used if C is bitmap
    ASSERT (!GB_IS_FULL (C)) ;      // GB_masker not used if C is full

    ASSERT (C->vdim == Z->vdim && C->vlen == Z->vlen) ;
    ASSERT (C->vdim == M->vdim && C->vlen == M->vlen) ;

    //--------------------------------------------------------------------------
    // determine the sparsity of R
    //--------------------------------------------------------------------------

    int R_sparsity = GB_masker_sparsity (C, M, Mask_comp, Z) ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Matrix R = NULL ;
    int64_t Rnvec, Rnvec_nonempty ;
    int64_t *Rp = NULL, *Rh = NULL ;
    int64_t *R_to_M = NULL, *R_to_C = NULL, *R_to_Z = NULL ;
    int R_ntasks = 0, TaskList_size = 0, R_nthreads ;
    GB_task_struct *TaskList = NULL ;

    //--------------------------------------------------------------------------
    // phase0: finalize the sparsity structure of R and the vectors of R
    //--------------------------------------------------------------------------

    // This phase is identical to phase0 of GB_add, except that Ch is never a
    // deep or shallow copy of Mh.

    info = GB_add_phase0 (
        // computed by by phase0:
        &Rnvec, &Rh, &R_to_M, &R_to_C, &R_to_Z, NULL, &R_sparsity,
        // original input:
        M, C, Z, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    GBURBLE ("masker:(%s:%s%s%s%s%s=%s) ",
        GB_sparsity_char (R_sparsity),
        GB_sparsity_char_matrix (C),
        Mask_struct ? "{" : "<",
        Mask_comp ? "!" : "",
        GB_sparsity_char_matrix (M),
        Mask_struct ? "}" : ">",
        GB_sparsity_char_matrix (Z)) ;

    //--------------------------------------------------------------------------
    // phase1: split R into tasks, and count entries in each vector of R
    //--------------------------------------------------------------------------

    if (R_sparsity == GxB_SPARSE || R_sparsity == GxB_HYPERSPARSE)
    {

        //----------------------------------------------------------------------
        // R is sparse or hypersparse: slice and analyze the R matrix
        //----------------------------------------------------------------------

        // phase1a: split R into tasks
        info = GB_ewise_slice (
            // computed by phase1a
            &TaskList, &TaskList_size, &R_ntasks, &R_nthreads,
            // computed by phase0:
            Rnvec, Rh, R_to_M, R_to_C, R_to_Z, false,
            // original input:
            M, C, Z, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory; free everything allocated by GB_add_phase0
            GB_FREE (Rh) ;
            GB_FREE (R_to_M) ;
            GB_FREE (R_to_C) ;
            GB_FREE (R_to_Z) ;
            return (info) ;
        }

        // count the number of entries in each vector of R
        info = GB_masker_phase1 (
            // computed or used by phase1:
            &Rp, &Rnvec_nonempty,
            // from phase1a:
            TaskList, R_ntasks, R_nthreads,
            // from phase0:
            Rnvec, Rh, R_to_M, R_to_C, R_to_Z,
            // original input:
            M, Mask_comp, Mask_struct, C, Z, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory; free everything allocated by GB_add_phase0
            GB_FREE (TaskList) ;
            GB_FREE (Rh) ;
            GB_FREE (R_to_M) ;
            GB_FREE (R_to_C) ;
            GB_FREE (R_to_Z) ;
            return (info) ;
        }

    }
    else
    { 

        //----------------------------------------------------------------------
        // R is bitmap or full: only determine how many threads to use
        //----------------------------------------------------------------------

        GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
        R_nthreads = GB_nthreads (M->vlen * M->vdim, chunk, nthreads_max) ;
    }

    //--------------------------------------------------------------------------
    // phase2: compute the entries (indices and values) in each vector of R
    //--------------------------------------------------------------------------

    // Rp and Rh are either freed by phase2, or transplanted into R.
    // Either way, they are not freed here.

    info = GB_masker_phase2 (
        // computed or used by phase2:
        &R, R_is_csc,
        // from phase1:
        Rp, Rnvec_nonempty,
        // from phase1a:
        TaskList, R_ntasks, R_nthreads,
        // from phase0:
        Rnvec, Rh, R_to_M, R_to_C, R_to_Z, R_sparsity,
        // original input:
        M, Mask_comp, Mask_struct, C, Z, Context) ;

    // if successful, Rh and Rp must not be freed; they are now R->h and R->p

    // free workspace
    GB_FREE (TaskList) ;
    GB_FREE (R_to_M) ;
    GB_FREE (R_to_C) ;
    GB_FREE (R_to_Z) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory; note that Rp and Rh are already freed
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (R, "R output for masker", GB0) ;
    (*Rhandle) = R ;
    return (GrB_SUCCESS) ;
}

