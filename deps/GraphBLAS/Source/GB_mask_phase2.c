//------------------------------------------------------------------------------
// GB_mask_phase2: phase2 for R = masker (M,C,Z)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_mask_phase2 computes R = masker (M,C,Z).  It is preceded first by
// GB_add_phase0, which computes the list of vectors of R to compute (Rh) and
// their location in C and Z (R_to_[CZ]).  Next, GB_mask_phase1 counts the
// entries in each vector R(:,j) and computes Rp.

// GB_mask_phase2 computes the pattern and values of each vector of R(:,j),
// fully in parallel.

// R, M, C, and Z can be standard sparse or hypersparse, as determined by
// GB_add_phase0.  All cases of the mask M are handled: present and not
// complemented, and present and complemented.  The mask is always present.

// This function either frees Rp and Rh, or transplants then into R, as R->p
// and R->h.  Either way, the caller must not free them.

#include "GB_mask.h"

GrB_Info GB_mask_phase2     // phase2 for R = masker (M,C,Z)
(
    GrB_Matrix *Rhandle,    // output matrix (unallocated on input)
    const bool R_is_csc,    // format of output matrix R
    // from phase1:
    const int64_t *GB_RESTRICT Rp,         // vector pointers for R
    const int64_t Rnvec_nonempty,       // # of non-empty vectors in R
    // tasks from phase0b:
    const GB_task_struct *GB_RESTRICT TaskList,    // array of structs
    const int ntasks,                           // # of tasks
    const int nthreads,                         // # of threads to use
    // analysis from phase0:
    const int64_t Rnvec,
    const int64_t *GB_RESTRICT Rh,
    const int64_t *GB_RESTRICT R_to_M,
    const int64_t *GB_RESTRICT R_to_C,
    const int64_t *GB_RESTRICT R_to_Z,
    // original input:
    const GrB_Matrix M,         // required mask
    const bool Mask_comp,
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix C,
    const GrB_Matrix Z,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Rp != NULL) ;
    ASSERT_MATRIX_OK (M, "M for mask phase2", GB0) ;
    ASSERT_MATRIX_OK (C, "C for mask phase2", GB0) ;
    ASSERT_MATRIX_OK (Z, "Z for mask phase2", GB0) ;
    ASSERT (C->vdim == Z->vdim && C->vlen == Z->vlen) ;
    ASSERT (C->vdim == M->vdim && C->vlen == M->vlen) ;
    ASSERT (C->type == Z->type) ;

    //--------------------------------------------------------------------------
    // allocate the output matrix R
    //--------------------------------------------------------------------------

    int64_t rnz = Rp [Rnvec] ;
    (*Rhandle) = NULL ;

    // R is hypersparse if both C and Z are hypersparse.
    // R acquires the same hyperatio as C.

    bool R_is_hyper = (Rh != NULL) ;

    // allocate the result R (but do not allocate R->p or R->h)
    GrB_Info info ;
    GrB_Matrix R = NULL ;
    GB_CREATE (&R, C->type, C->vlen, C->vdim, GB_Ap_null, R_is_csc,
        GB_SAME_HYPER_AS (R_is_hyper), C->hyper_ratio, Rnvec, rnz, true,
        Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory; caller must free R_to_M, R_to_C, R_to_Z
        GB_FREE_MEMORY (Rp, GB_IMAX (2, Rnvec+1), sizeof (int64_t)) ;
        GB_FREE_MEMORY (Rh, Rnvec, sizeof (int64_t)) ;
        return (info) ;
    }

    // add Rp as the vector pointers for R, from GB_mask_phase1
    R->p = (int64_t *) Rp ;

    // add Rh as the hypersparse list for R, from GB_add_phase0
    if (R_is_hyper)
    { 
        R->h = (int64_t *) Rh ;
        R->nvec = Rnvec ;
    }

    // now Rp and Rh have been transplanted into R, so they must not be freed.

    R->nvec_nonempty = Rnvec_nonempty ;
    R->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // generic worker
    //--------------------------------------------------------------------------

    #define GB_PHASE_2_OF_2
    #include "GB_mask_template.c"

    //--------------------------------------------------------------------------
    // prune empty vectors from Rh
    //--------------------------------------------------------------------------

    info = GB_hypermatrix_prune (R, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_MATRIX_FREE (&R) ;
        return (info) ;
    }

#if 0
    // see GB_hypermatrix_prune
    if (R_is_hyper && R->nvec_nonempty < Rnvec)
    {
        // create new Rp_new and Rh_new arrays, with no empty vectors
        int64_t *GB_RESTRICT Rp_new = NULL ;
        int64_t *GB_RESTRICT Rh_new = NULL ;
        int64_t nvec_new ;
        info = GB_hyper_prune (&Rp_new, &Rh_new, &nvec_new, R->p, R->h, Rnvec,
            Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_MATRIX_FREE (&R) ;
            return (info) ;
        }
        // transplant the new hyperlist into R
        GB_FREE_MEMORY (R->p, GB_IMAX (2, Rnvec+1), sizeof (int64_t)) ;
        GB_FREE_MEMORY (R->h, Rnvec, sizeof (int64_t)) ;
        R->p = Rp_new ;
        R->h = Rh_new ;
        R->nvec = nvec_new ;
        R->plen = nvec_new ;
        ASSERT (R->nvec == R->nvec_nonempty) ;
    }
#endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // caller must free R_to_M, R_to_C, and R_to_Z, but not Rp or Rh
    ASSERT_MATRIX_OK (R, "R output for mask phase2", GB0) ;
    (*Rhandle) = R ;
    return (GrB_SUCCESS) ;
}

