//------------------------------------------------------------------------------
// GB_add.h: definitiions for GB_add and related functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_ADD_H
#define GB_ADD_H
#include "GB.h"

GrB_Info GB_add             // C=A+B or C<M>=A+B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix M,     // optional mask for C, unused if NULL
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    GB_Context Context
) ;

GrB_Info GB_add_phase0          // find vectors in C for C=A+B or C<M>=A+B
(
    int64_t *p_Cnvec,           // # of vectors to compute in C
    int64_t *restrict *Ch_handle,        // Ch: size Cnvec, or NULL
    int64_t *restrict *C_to_M_handle,    // C_to_M: size Cnvec, or NULL
    int64_t *restrict *C_to_A_handle,    // C_to_A: size Cnvec, or NULL
    int64_t *restrict *C_to_B_handle,    // C_to_B: of size Cnvec, or NULL
    bool *p_Ch_is_Mh,           // if true, then Ch == Mh
    const GrB_Matrix M,         // optional mask, may be NULL; not complemented
    const GrB_Matrix A,         // standard, hypersparse, slice, or hyperslice
    const GrB_Matrix B,         // standard or hypersparse; never a slice
    GB_Context Context
) ;

GrB_Info GB_add_phase1                  // count nnz in each C(:,j)
(
    int64_t **Cp_handle,                // output of size Cnvec+1
    int64_t *Cnvec_nonempty,            // # of non-empty vectors in C
    const bool A_and_B_are_disjoint,    // if true, then A and B are disjoint
    // tasks from phase0b:
    GB_task_struct *restrict TaskList,      // array of structs
    const int ntasks,                       // # of tasks
    const int nthreads,                     // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *restrict Ch,
    const int64_t *restrict C_to_M,
    const int64_t *restrict C_to_A,
    const int64_t *restrict C_to_B,
    const bool Ch_is_Mh,                // if true, then Ch == M->h
    // original input:
    const GrB_Matrix M,                 // optional mask, may be NULL
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

GrB_Info GB_add_phase2      // C=A+B or C<M>=A+B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_BinaryOp op,  // op to perform C = op (A,B), or NULL if no op
    // from phase1:
    const int64_t *restrict Cp,         // vector pointers for C
    const int64_t Cnvec_nonempty,       // # of non-empty vectors in C
    // tasks from phase0b:
    const GB_task_struct *restrict TaskList,    // array of structs
    const int ntasks,                           // # of tasks
    const int nthreads,                         // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *restrict Ch,
    const int64_t *restrict C_to_M,
    const int64_t *restrict C_to_A,
    const int64_t *restrict C_to_B,
    const bool Ch_is_Mh,        // if true, then Ch == M->h
    // original input:
    const GrB_Matrix M,         // optional mask, may be NULL
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

#endif
