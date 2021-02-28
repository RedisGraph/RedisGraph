//------------------------------------------------------------------------------
// GB_masker: R = masker (M, C, Z)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_masker (R, M, C, Z), does R=C ; R<M>=Z.  No typecasting is performed.
// The operation is similar to both R=C+Z via GB_add and R=C.*Z via GB_emult,
// depending on the value of the mask.

// Let R be the result of the mask.  In the caller, R is written back into the
// final C matrix, but in GB_masker, C is a read-only matrix.  Consider the
// following table, where "add" is the result of C+Z, an "emult" is the result
// of C.*Z.

//                                      R = masker (M,C,Z)

// C(i,j)   Z(i,j)  add     emult       M(i,j)=1    M(i,j)=0

// ------   ------  ------  ------      --------    --------

//  cij     zij     cij+zij cij*zij     zij         cij

//   -      zij     zij     -           zij         -

//  cij     -       cij     -           -           cij

//   -      -       -       -           -           -

// Half of the results are like C.*Z using the FIRST operator, and the
// other are the same as C+Z using the SECOND operator:

//  cij     zij     cij+zij cij*zij     2nd(C+Z)    1st(C.*Z)

//   -      zij     zij     -           2nd(C+Z)    1st(C.*Z)

//  cij     -       cij     -           1st(C.*Z)   2nd(C+Z)

//   -      -       -       -           1st(C.*Z)   2nd(C+Z)

// As a result, GB_masker is very similar to GB_add and GB_emult.  The
// vectors that appear in R are bounded by the set union of C and Z, just
// like GB_add when the mask is *not* present.  The pattern of R is bounded
// by the pattern of C+Z, also ignoring the mask.

#include "GB_mask.h"
#include "GB_add.h"

GrB_Info GB_masker          // R = masker (M, C, Z)
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

    GBBURBLE ("mask ") ;

    ASSERT (Rhandle != NULL) ;
    ASSERT_MATRIX_OK (M, "M for masker", GB0) ;
    ASSERT_MATRIX_OK (C, "C for masker", GB0) ;
    ASSERT_MATRIX_OK (Z, "Z for masker", GB0) ;
    ASSERT (C->vdim == Z->vdim && C->vlen == Z->vlen) ;
    ASSERT (C->vdim == M->vdim && C->vlen == M->vlen) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (Z)) ; ASSERT (!GB_ZOMBIES (Z)) ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Matrix R = NULL ;
    int64_t Rnvec, Rnvec_nonempty ;
    int64_t *Rp = NULL, *Rh = NULL ;
    int64_t *R_to_M = NULL, *R_to_C = NULL, *R_to_Z = NULL ;
    int ntasks, max_ntasks, nthreads ;
    GB_task_struct *TaskList = NULL ;

    //--------------------------------------------------------------------------
    // phase0: determine the vectors in R = C+Z
    //--------------------------------------------------------------------------

    // This phase is identical to phase0 of GB_add, except that Ch is never a
    // deep or shallow copy of Mh.

    GrB_Info info = GB_add_phase0 (
        // computed by by phase0:
        &Rnvec, &Rh, &R_to_M, &R_to_C, &R_to_Z, NULL,
        // original input:
        M, C, Z, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // phase0b: split C into tasks for phase1 and phase2
    //--------------------------------------------------------------------------

    info = GB_ewise_slice (
        // computed by phase0b
        &TaskList, &max_ntasks, &ntasks, &nthreads,
        // computed by phase0:
        Rnvec, Rh, R_to_M, R_to_C, R_to_Z, false,
        // original input:
        M, C, Z, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory; free everything allocated by GB_add_phase0
        GB_FREE_MEMORY (Rh,     Rnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (R_to_M, Rnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (R_to_C, Rnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (R_to_Z, Rnvec, sizeof (int64_t)) ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // phase1: count the number of entries in each vector of R
    //--------------------------------------------------------------------------

    info = GB_mask_phase1 (
        // computed or used by phase1:
        &Rp, &Rnvec_nonempty,
        // from phase0b:
        TaskList, ntasks, nthreads,
        // from phase0:
        Rnvec, Rh, R_to_M, R_to_C, R_to_Z,
        // original input:
        M, Mask_comp, Mask_struct, C, Z, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory; free everything allocated by GB_add_phase0
        GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;
        GB_FREE_MEMORY (Rh,     Rnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (R_to_M, Rnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (R_to_C, Rnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (R_to_Z, Rnvec, sizeof (int64_t)) ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // phase2: compute the entries (indices and values) in each vector of R
    //--------------------------------------------------------------------------

    // Rp and Rh are either freed by phase2, or transplanted into R.
    // Either way, they are not freed here.

    info = GB_mask_phase2 (
        // computed or used by phase2:
        &R, R_is_csc,
        // from phase1:
        Rp, Rnvec_nonempty,
        // from phase0b:
        TaskList, ntasks, nthreads,
        // from phase0:
        Rnvec, Rh, R_to_M, R_to_C, R_to_Z,
        // original input:
        M, Mask_comp, Mask_struct, C, Z, Context) ;

    // free workspace
    GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;
    GB_FREE_MEMORY (R_to_M, Rnvec, sizeof (int64_t)) ;
    GB_FREE_MEMORY (R_to_C, Rnvec, sizeof (int64_t)) ;
    GB_FREE_MEMORY (R_to_Z, Rnvec, sizeof (int64_t)) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory; note that Rp and Rh are already freed
        return (info) ;
    }

    // if successful, Rh and Rp must not be freed; they are now R->h and R->p

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (R, "R output for masker", GB0) ;
    (*Rhandle) = R ;
    return (GrB_SUCCESS) ;
}

