//------------------------------------------------------------------------------
// GB_add.h: definitiions for GB_add and related functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_ADD_H
#define GB_ADD_H
#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_add             // C=A+B, C<M>=A+B, or C<!M>=A+B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix M,     // optional mask for C, unused if NULL
    const bool Mask_struct, // if true, use the only structure of M
    const bool Mask_comp,   // if true, use !M
    bool *mask_applied,     // if true, the mask was applied
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    GB_Context Context
) ;

GrB_Info GB_add_phase0          // find vectors in C for C=A+B or C<M>=A+B
(
    int64_t *p_Cnvec,           // # of vectors to compute in C
    int64_t *GB_RESTRICT *Ch_handle,        // Ch: size Cnvec, or NULL
    int64_t *GB_RESTRICT *C_to_M_handle,    // C_to_M: size Cnvec, or NULL
    int64_t *GB_RESTRICT *C_to_A_handle,    // C_to_A: size Cnvec, or NULL
    int64_t *GB_RESTRICT *C_to_B_handle,    // C_to_B: of size Cnvec, or NULL
    bool *p_Ch_is_Mh,           // if true, then Ch == Mh
    int *C_sparsity,            // sparsity structure of C
    const GrB_Matrix M,         // optional mask, may be NULL; not complemented
    const GrB_Matrix A,         // first input matrix
    const GrB_Matrix B,         // second input matrix
    GB_Context Context
) ;

GrB_Info GB_add_phase1                  // count nnz in each C(:,j)
(
    int64_t **Cp_handle,                // output of size Cnvec+1
    int64_t *Cnvec_nonempty,            // # of non-empty vectors in C
    const bool A_and_B_are_disjoint,    // if true, then A and B are disjoint
    // tasks from phase0b:
    GB_task_struct *GB_RESTRICT TaskList,   // array of structs
    const int C_ntasks,                 // # of tasks
    const int C_nthreads,               // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *GB_RESTRICT Ch,
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    const bool Ch_is_Mh,                // if true, then Ch == M->h
    // original input:
    const GrB_Matrix M,             // optional mask, may be NULL
    const bool Mask_struct,         // if true, use the only structure of M
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

GrB_Info GB_add_phase2      // C=A+B, C<M>=A+B, or C<!M>=A+B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_BinaryOp op,  // op to perform C = op (A,B), or NULL if no op
    // from phase1:
    const int64_t *GB_RESTRICT Cp,  // vector pointers for C
    const int64_t Cnvec_nonempty,   // # of non-empty vectors in C
    // tasks from phase1a:
    const GB_task_struct *GB_RESTRICT TaskList,    // array of structs
    const int C_ntasks,         // # of tasks
    const int C_nthreads,       // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *GB_RESTRICT Ch,
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    const bool Ch_is_Mh,        // if true, then Ch == M->h
    const int C_sparsity,
    // original input:
    const GrB_Matrix M,         // optional mask, may be NULL
    const bool Mask_struct,     // if true, use the only structure of M
    const bool Mask_comp,       // if true, use !M
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

int GB_add_sparsity         // return the sparsity structure for C
(
    // output:
    bool *apply_mask,       // if true then mask will be applied
    // input:
    const GrB_Matrix M,     // optional mask for C, unused if NULL
    const bool Mask_comp,   // if true, use !M
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B      // input B matrix
) ;

#endif
