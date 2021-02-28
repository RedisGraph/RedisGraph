//------------------------------------------------------------------------------
// GB_mask: definitions for GB_mask and related functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_MASK_H
#define GB_MASK_H
#include "GB.h"

GrB_Info GB_mask                // C<M> = Z
(
    GrB_Matrix C_result,        // both input C and result matrix
    const GrB_Matrix M,         // optional mask matrix, can be NULL
    GrB_Matrix *Zhandle,        // Z = results of computation, perhaps shallow.
                                // Z is freed when done.
    const bool C_replace,       // true if clear(C) to be done first
    const bool Mask_comp,       // true if M is to be complemented
    const bool Mask_struct,     // if true, use the only structure of M
    GB_Context Context
) ;

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
) ;

GrB_Info GB_mask_phase1                 // count nnz in each R(:,j)
(
    int64_t **Rp_handle,                // output of size Rnvec+1
    int64_t *Rnvec_nonempty,            // # of non-empty vectors in R
    // tasks from phase0b:
    GB_task_struct *GB_RESTRICT TaskList,      // array of structs
    const int ntasks,                       // # of tasks
    const int nthreads,                     // # of threads to use
    // analysis from phase0:
    const int64_t Rnvec,
    const int64_t *GB_RESTRICT Rh,
    const int64_t *GB_RESTRICT R_to_M,
    const int64_t *GB_RESTRICT R_to_C,
    const int64_t *GB_RESTRICT R_to_Z,
    // original input:
    const GrB_Matrix M,                 // required mask
    const bool Mask_comp,               // if true, then M is complemented
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix C,
    const GrB_Matrix Z,
    GB_Context Context
) ;

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
) ;

#endif

