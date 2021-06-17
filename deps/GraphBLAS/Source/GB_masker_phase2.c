//------------------------------------------------------------------------------
// GB_masker_phase2: phase2 for R = masker (C,M,Z)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_masker_phase2 computes R = masker (C,M,Z).  It is preceded first by
// GB_add_phase0, which computes the list of vectors of R to compute (Rh) and
// their location in C and Z (R_to_[CZ]).  Next, GB_masker_phase1 counts the
// entries in each vector R(:,j) and computes Rp.

// GB_masker_phase2 computes the pattern and values of each vector of R(:,j),
// entirely in parallel.

// R, M, C, and Z can be standard sparse or hypersparse, as determined by
// GB_add_phase0.  All cases of the mask M are handled: present and not
// complemented, and present and complemented.  The mask is always present.

// This function either frees Rp and Rh, or transplants then into R, as R->p
// and R->h.  Either way, the caller must not free them.

#include "GB_mask.h"
#include "GB_ek_slice.h"
#include "GB_unused.h"

#undef  GB_FREE_WORK
#define GB_FREE_WORK                                                    \
{                                                                       \
    GB_ek_slice_free (&pstart_Cslice, &kfirst_Cslice, &klast_Cslice) ;  \
    GB_ek_slice_free (&pstart_Mslice, &kfirst_Mslice, &klast_Mslice) ;  \
}

#undef  GB_FREE_ALL
#define GB_FREE_ALL         \
{                           \
    GB_FREE_WORK ;          \
    GB_Matrix_free (&R) ;   \
}

GrB_Info GB_masker_phase2           // phase2 for R = masker (C,M,Z)
(
    GrB_Matrix *Rhandle,            // output matrix (unallocated on input)
    const bool R_is_csc,            // format of output matrix R
    // from phase1:
    const int64_t *GB_RESTRICT Rp,  // vector pointers for R
    const int64_t Rnvec_nonempty,   // # of non-empty vectors in R
    // tasks from phase1a:
    const GB_task_struct *GB_RESTRICT TaskList,     // array of structs
    const int R_ntasks,               // # of tasks
    const int R_nthreads,             // # of threads to use
    // analysis from phase0:
    const int64_t Rnvec,
    const int64_t *GB_RESTRICT Rh,
    const int64_t *GB_RESTRICT R_to_M,
    const int64_t *GB_RESTRICT R_to_C,
    const int64_t *GB_RESTRICT R_to_Z,
    const int R_sparsity,
    // original input:
    const GrB_Matrix M,             // required mask
    const bool Mask_comp,           // if true, then M is complemented
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix C,
    const GrB_Matrix Z,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (M, "M for mask phase2", GB0) ;
    ASSERT (!GB_ZOMBIES (M)) ; 
    ASSERT (!GB_JUMBLED (M)) ;
    ASSERT (!GB_PENDING (M)) ; 

    ASSERT_MATRIX_OK (C, "C for mask phase2", GB0) ;
    ASSERT (!GB_ZOMBIES (C)) ; 
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_PENDING (C)) ; 

    ASSERT_MATRIX_OK (Z, "Z for mask phase2", GB0) ;
    ASSERT (!GB_ZOMBIES (Z)) ; 
    ASSERT (!GB_JUMBLED (Z)) ;
    ASSERT (!GB_PENDING (Z)) ; 

    ASSERT (!GB_IS_BITMAP (C)) ;        // not used if C is bitmap

    ASSERT (C->vdim == Z->vdim && C->vlen == Z->vlen) ;
    ASSERT (C->vdim == M->vdim && C->vlen == M->vlen) ;
    ASSERT (C->type == Z->type) ;

    int64_t *pstart_Cslice = NULL, *kfirst_Cslice = NULL, *klast_Cslice = NULL ;
    int64_t *pstart_Mslice = NULL, *kfirst_Mslice = NULL, *klast_Mslice = NULL ;

    //--------------------------------------------------------------------------
    // allocate the output matrix R
    //--------------------------------------------------------------------------

    bool R_is_hyper = (R_sparsity == GxB_HYPERSPARSE) ;
    bool R_is_sparse_or_hyper = (R_sparsity == GxB_SPARSE) || R_is_hyper ;
    ASSERT (R_is_sparse_or_hyper == (Rp != NULL)) ;
    ASSERT (R_is_hyper == (Rh != NULL)) ;

    int64_t rnz = (R_is_sparse_or_hyper) ? Rp [Rnvec] : C->vlen*C->vdim ;
    (*Rhandle) = NULL ;

    // allocate the result R (but do not allocate R->p or R->h)
    GrB_Matrix R = NULL ;
    GrB_Info info = GB_new_bix (&R, // any sparsity, new header
        C->type, C->vlen, C->vdim, GB_Ap_null, R_is_csc,
        R_sparsity, true, C->hyper_switch, Rnvec, rnz, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory; caller must free R_to_M, R_to_C, R_to_Z
        GB_FREE (Rp) ;
        GB_FREE (Rh) ;
        return (info) ;
    }

    // add Rp as the vector pointers for R, from GB_masker_phase1
    if (R_is_sparse_or_hyper)
    { 
        R->nvec_nonempty = Rnvec_nonempty ;
        R->p = (int64_t *) Rp ;
    }

    // add Rh as the hypersparse list for R, from GB_add_phase0
    if (R_is_hyper)
    { 
        R->h = (int64_t *) Rh ;
        R->nvec = Rnvec ;
    }

    // now Rp and Rh have been transplanted into R, so they must not be freed.
    R->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // generic worker
    //--------------------------------------------------------------------------

    #define GB_PHASE_2_OF_2
    #include "GB_masker_template.c"

    //--------------------------------------------------------------------------
    // prune empty vectors from Rh
    //--------------------------------------------------------------------------

    GB_OK (GB_hypermatrix_prune (R, Context)) ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    // caller must free R_to_M, R_to_C, and R_to_Z, but not Rp or Rh
    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (R, "R output for mask phase2", GB0) ;
    ASSERT (!GB_ZOMBIES (R)) ; 
    ASSERT (!GB_JUMBLED (R)) ;
    ASSERT (!GB_PENDING (R)) ; 
    (*Rhandle) = R ;
    return (GrB_SUCCESS) ;
}

