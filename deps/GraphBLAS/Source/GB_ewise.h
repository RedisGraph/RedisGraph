//------------------------------------------------------------------------------
// GB_ewise.h: definitions for GB_ewise
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_EWISE_H
#define GB_EWISE_H
#include "GB.h"

GrB_Info GB_ewise                   // C<M> = accum (C, A+B) or A.*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, complement the mask M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '+' for C=A+B, or .* for A.*B
    const GrB_Matrix A,             // input matrix
    bool A_transpose,               // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    bool B_transpose,               // if true, use B' instead of B
    bool eWiseAdd,                  // if true, do set union (like A+B),
                                    // otherwise do intersection (like A.*B)
    const bool is_eWiseUnion,       // if true, eWiseUnion, else eWiseAdd
    const GrB_Scalar alpha,         // alpha and beta ignored for eWiseAdd,
    const GrB_Scalar beta,          // nonempty scalars for GxB_eWiseUnion
    GB_Context Context
) ;

void GB_ewise_generic       // generic ewise
(
    // input/output:
    GrB_Matrix C,           // output matrix, static header
    // input:
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    // tasks from phase1a:
    const GB_task_struct *restrict TaskList,  // array of structs
    const int C_ntasks,                         // # of tasks
    const int C_nthreads,                       // # of threads to use
    // analysis from phase0:
    const int64_t *restrict C_to_M,
    const int64_t *restrict C_to_A,
    const int64_t *restrict C_to_B,
    const int C_sparsity,
    // from GB_emult_sparsity or GB_add_sparsity:
    const int ewise_method,
    // from GB_emult_04 and GB_emult_02:
    const int64_t *restrict Cp_kfirst,
    // to slice M, A, and/or B,
    const int64_t *M_ek_slicing, const int M_ntasks, const int M_nthreads,
    const int64_t *A_ek_slicing, const int A_ntasks, const int A_nthreads,
    const int64_t *B_ek_slicing, const int B_ntasks, const int B_nthreads,
    // original input:
    const GrB_Matrix M,             // optional mask, may be NULL
    const bool Mask_struct,         // if true, use the only structure of M
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

#endif
