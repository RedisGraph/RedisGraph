//------------------------------------------------------------------------------
// GB_AxB_saxpy3.h: definitions for C=A*B saxpy3 method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_AxB_saxpy3 method uses a mix of Gustavson's method and the Hash method,
// combining the two for any given C=A*B computation.

#ifndef GB_AXB_SAXPY3_H
#define GB_AXB_SAXPY3_H
#include "GB.h"

GrB_Info GB_AxB_saxpy3              // C = A*B using Gustavson+Hash
(
    GrB_Matrix *Chandle,            // output matrix (not done in-place)
    int C_sparsity,                 // construct C as sparse or hypersparse
    const GrB_Matrix M_input,       // optional mask matrix
    const bool Mask_comp_input,     // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, then mask was applied
    GrB_Desc_Value AxB_method,      // Default, Gustavson, or Hash
    const int do_sort,              // if nonzero, try to sort in saxpy3
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// functions for the Hash method for C=A*B
//------------------------------------------------------------------------------

// initial hash function, for where to place the integer i in the hash table.
// hash_bits is a bit mask to compute the result modulo the hash table size,
// which is always a power of 2.  The function is (i*257) & (hash_bits).
#define GB_HASHF(i) ((((i) << 8) + (i)) & (hash_bits))

// rehash function, for subsequent hash lookups if the initial hash function
// refers to a hash entry that is already occupied.  Linear probing is used,
// so the function does not currently depend on i.  On input, hash is equal
// to the current value of the hash function, and on output, hash is set to
// the new hash value.
#define GB_REHASH(hash,i) hash = ((hash + 1) & (hash_bits))

// The hash functions and their parameters are modified from this paper:

// [2] Yusuke Nagasaka, Satoshi Matsuoka, Ariful Azad, and Aydin Buluc. 2018.
// High-Performance Sparse Matrix-Matrix Products on Intel KNL and Multicore
// Architectures. In Proc. 47th Intl. Conf. on Parallel Processing (ICPP '18).
// Association for Computing Machinery, New York, NY, USA, Article 34, 1–10.
// DOI:https://doi.org/10.1145/3229710.3229720

// The hash function in that paper is (i*107)&(hash_bits).  Here, the term
// 107 is replaced with 257 to allow for a faster hash function computation.

//------------------------------------------------------------------------------
// GB_saxpy3task_struct: task descriptor for GB_AxB_saxpy3
//------------------------------------------------------------------------------

// A coarse task computes C(:,j1:j2) = A*B(:,j1:j2), for a contiguous set of
// vectors j1:j2.  A coarse taskid is denoted byTaskList [taskid].vector == -1,
// kfirst = TaskList [taskid].start, and klast = TaskList [taskid].end, and
// where j1 = GBH (Bh, kstart) and likewise for j2.  No summation is needed for
// the final result of each coarse task.

// A fine taskid computes A*B(k1:k2,j) for a single vector C(:,j), for a
// contiguous range k1:k2, where kk = Tasklist[taskid].vector (which is >= 0),
// k1 = Bi [TaskList [taskid].start], k2 = Bi [TaskList [taskid].end].  It sums
// its computations in a hash table shared by all fine tasks that compute
// C(:,j), via atomics.  The vector index j is GBH (Bh, kk).

// Both tasks use a hash table allocated uniquely for the task, in Hi, Hf, and
// Hx.  The size of the hash table is determined by the maximum # of flops
// needed to compute any vector in C(:,j1:j2) for a coarse task, or the entire
// computation of the single vector in a fine task.  For the Hash method, the
// table has a size that is twice the smallest a power of 2 larger than the
// flop count.  If this size is a significant fraction of C->vlen, then the
// Hash method is not used, and Gustavson's method is used, with the hash size
// is set to C->vlen.

typedef struct
{
    int64_t start ;     // starting vector for coarse task, p for fine task
    int64_t end ;       // ending vector for coarse task, p for fine task
    int64_t vector ;    // -1 for coarse task, vector j for fine task
    int64_t hsize ;     // size of hash table
    int64_t *Hi ;       // Hi array for hash table (coarse hash tasks only)
    GB_void *Hf ;       // Hf array for hash table (int8_t or int64_t)
    GB_void *Hx ;       // Hx array for hash table
    int64_t my_cjnz ;   // # entries in C(:,j) found by this fine task
    int leader ;        // leader fine task for the vector C(:,j)
    int team_size ;     // # of fine tasks in the team for vector C(:,j)
}
GB_saxpy3task_struct ;

//------------------------------------------------------------------------------
// GB_AxB_saxpy3_flopcount:  compute flops for GB_AxB_saxpy3
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_AxB_saxpy3_flopcount
(
    int64_t *Mwork,             // amount of work to handle the mask M
    int64_t *Bflops,            // size B->nvec+1 and all zero
    const GrB_Matrix M,         // optional mask matrix
    const bool Mask_comp,       // if true, mask is complemented
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// GB_AxB_saxpy3_symbolic: symbolic analysis for GB_AxB_saxpy3
//------------------------------------------------------------------------------

void GB_AxB_saxpy3_symbolic
(
    GrB_Matrix C,               // Cp is computed for coarse tasks
    const GrB_Matrix M,         // mask matrix M
    const bool Mask_comp,       // M complemented, or not
    const bool Mask_struct,     // M structural, or not
    const bool M_dense_in_place,
    const GrB_Matrix A,         // A matrix; only the pattern is accessed
    const GrB_Matrix B,         // B matrix; only the pattern is accessed
    GB_saxpy3task_struct *TaskList,     // list of tasks, and workspace
    int ntasks,                 // total number of tasks
    int nfine,                  // number of fine tasks
    int nthreads                // number of threads
) ;

//------------------------------------------------------------------------------
// GB_AxB_saxpy3_cumsum: cumulative sum of C->p for GB_AxB_saxpy3
//------------------------------------------------------------------------------

void GB_AxB_saxpy3_cumsum
(
    GrB_Matrix C,               // finalize C->p
    GB_saxpy3task_struct *TaskList, // list of tasks, and workspace
    int nfine,                  // number of fine tasks
    double chunk,               // chunk size
    int nthreads                // number of threads
) ;

//------------------------------------------------------------------------------
// GB_AxB_saxpy3_slice_balanced: create balanced parallel tasks for saxpy3
//------------------------------------------------------------------------------

GrB_Info GB_AxB_saxpy3_slice_balanced
(
    // inputs
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix M,             // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    GrB_Desc_Value AxB_method,      // Default, Gustavson, or Hash
    // outputs
    GB_saxpy3task_struct **TaskList_handle,
    bool *apply_mask,               // if true, apply M during sapxy3
    bool *M_dense_in_place,         // if true, use M in-place
    int *ntasks,                    // # of tasks created (coarse and fine)
    int *nfine,                     // # of fine tasks created
    int *nthreads,                  // # of threads to use
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// GB_AxB_saxpy3_slice_quick: create a single sequential task for saxpy3
//------------------------------------------------------------------------------

GrB_Info GB_AxB_saxpy3_slice_quick
(
    // inputs
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    // outputs
    GB_saxpy3task_struct **TaskList_handle,
    int *ntasks,                    // # of tasks created (coarse and fine)
    int *nfine,                     // # of fine tasks created
    int *nthreads,                  // # of threads to use
    GB_Context Context
) ;

#endif

