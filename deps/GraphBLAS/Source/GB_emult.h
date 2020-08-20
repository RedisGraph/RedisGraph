//------------------------------------------------------------------------------
// GB_emult.h: definitions for GB_emult
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_EMULT_H
#define GB_EMULT_H
#include "GB.h"

GrB_Info GB_emult           // C=A.*B or C<M>=A.*B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix M,     // optional mask, unused if NULL.  Not complemented
    const bool Mask_struct, // if true, use the only structure of M
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    GB_Context Context
) ;

GrB_Info GB_emult_phase0        // find vectors in C for C=A.*B or C<M>=A.*B
(
    int64_t *p_Cnvec,           // # of vectors to compute in C
    const int64_t *GB_RESTRICT *Ch_handle,  // Ch is M->h, A->h, B->h, or NULL
    int64_t *GB_RESTRICT *C_to_M_handle,    // C_to_M: size Cnvec, or NULL
    int64_t *GB_RESTRICT *C_to_A_handle,    // C_to_A: size Cnvec, or NULL
    int64_t *GB_RESTRICT *C_to_B_handle,    // C_to_B: size Cnvec, or NULL
    // original input:
    const GrB_Matrix M,         // optional mask, may be NULL
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

GrB_Info GB_emult_phase1                // count nnz in each C(:,j)
(
    int64_t *GB_RESTRICT *Cp_handle,       // output of size Cnvec+1
    int64_t *Cnvec_nonempty,            // # of non-empty vectors in C
    // tasks from phase0b:
    GB_task_struct *GB_RESTRICT TaskList,  // array of structs
    const int ntasks,                   // # of tasks
    const int nthreads,                 // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *GB_RESTRICT Ch,         // Ch is NULL, or shallow pointer
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    // original input:
    const GrB_Matrix M,                 // optional mask, may be NULL
    const bool Mask_struct, // if true, use the only structure of M
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

GrB_Info GB_emult_phase2                // C=A.*B or C<M>=A.*B
(
    GrB_Matrix *Chandle,                // output matrix
    const GrB_Type ctype,               // type of output matrix C
    const bool C_is_csc,                // format of output matrix C
    const GrB_BinaryOp op,              // op to perform C = op (A,B)
    // from phase1:
    const int64_t *GB_RESTRICT Cp,         // vector pointers for C
    const int64_t Cnvec_nonempty,       // # of non-empty vectors in C
    // tasks from phase0b:
    const GB_task_struct *GB_RESTRICT TaskList,  // array of structs
    const int ntasks,                         // # of tasks
    const int nthreads,                       // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *GB_RESTRICT Ch,         // Ch is NULL, or a shallow pointer
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    // original input:
    const GrB_Matrix M,                 // optional mask, may be NULL
    const bool Mask_struct, // if true, use the only structure of M
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

#endif
