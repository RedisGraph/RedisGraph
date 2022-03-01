//------------------------------------------------------------------------------
// GB_ek_slice.h: slice the entries and vectors of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_EK_SLICE_H
#define GB_EK_SLICE_H
#include "GB.h"

//------------------------------------------------------------------------------
// GB_ek_slice_ntasks: determine # of threads and tasks to use for GB_ek_slice
//------------------------------------------------------------------------------

static inline void GB_ek_slice_ntasks
(
    // output
    int *nthreads,              // # of threads to use for GB_ek_slice
    int *ntasks,                // # of tasks to create for GB_ek_slice
    // input
    GrB_Matrix A,               // matrix to slice
    int ntasks_per_thread,      // # of tasks per thread
    double work,                // total work to do
    double chunk,               // give each thread at least this much work
    int nthreads_max            // max # of threads to use
)
{
    int64_t anz = GB_nnz_held (A) ;
    if (anz == 0)
    {
        (*nthreads) = 1 ;
        (*ntasks) = 1 ;
    }
    else
    {
        (*nthreads) = GB_nthreads (work, chunk, nthreads_max) ;
        (*ntasks) = (*nthreads == 1) ? 1 : ((ntasks_per_thread) * (*nthreads)) ;
        (*ntasks) = GB_IMIN (*ntasks, anz) ;
        (*ntasks) = GB_IMAX (*ntasks, 1) ;
    }
}

//------------------------------------------------------------------------------
// GB_SLICE_MATRIX: slice a single matrix using GB_ek_slice
//------------------------------------------------------------------------------

#define GB_SLICE_MATRIX_WORK(X,NTASKS_PER_THREAD,chunk,work)                  \
    GB_ek_slice_ntasks (&(X ## _nthreads), &(X ## _ntasks), X,                \
        NTASKS_PER_THREAD, work, chunk, nthreads_max) ;                       \
    GB_WERK_PUSH (X ## _ek_slicing, 3*(X ## _ntasks)+1, int64_t) ;            \
    if (X ## _ek_slicing == NULL)                                             \
    {                                                                         \
        /* out of memory */                                                   \
        GB_FREE_ALL ;                                                         \
        return (GrB_OUT_OF_MEMORY) ;                                          \
    }                                                                         \
    GB_ek_slice (X ## _ek_slicing, X, X ## _ntasks) ;                         \
    const int64_t *kfirst_ ## X ## slice = X ## _ek_slicing ;                 \
    const int64_t *klast_  ## X ## slice = X ## _ek_slicing + X ## _ntasks ;  \
    const int64_t *pstart_ ## X ## slice = X ## _ek_slicing + X ## _ntasks*2 ;

#define GB_SLICE_MATRIX(X,NTASKS_PER_THREAD,chunk)                            \
    GB_ek_slice_ntasks (&(X ## _nthreads), &(X ## _ntasks), X,                \
        NTASKS_PER_THREAD, GB_nnz_held (X) + X->nvec, chunk, nthreads_max) ;  \
    GB_WERK_PUSH (X ## _ek_slicing, 3*(X ## _ntasks)+1, int64_t) ;            \
    if (X ## _ek_slicing == NULL)                                             \
    {                                                                         \
        /* out of memory */                                                   \
        GB_FREE_ALL ;                                                         \
        return (GrB_OUT_OF_MEMORY) ;                                          \
    }                                                                         \
    GB_ek_slice (X ## _ek_slicing, X, X ## _ntasks) ;                         \
    const int64_t *kfirst_ ## X ## slice = X ## _ek_slicing ;                 \
    const int64_t *klast_  ## X ## slice = X ## _ek_slicing + X ## _ntasks ;  \
    const int64_t *pstart_ ## X ## slice = X ## _ek_slicing + X ## _ntasks*2 ;

//------------------------------------------------------------------------------
// GB_ek_slice prototypes
//------------------------------------------------------------------------------

// Slice the entries of a matrix or vector into ntasks slices.

// Task t does entries pstart_slice [t] to pstart_slice [t+1]-1 and
// vectors kfirst_slice [t] to klast_slice [t].  The first and last vectors
// may be shared with prior slices and subsequent slices.

// On input, ntasks must be <= nnz (A), unless nnz (A) is zero.  In that
// case, ntasks must be 1.

void GB_ek_slice            // slice a matrix
(
    // output:
    int64_t *restrict A_ek_slicing,  // size 3*ntasks+1
    // input:
    GrB_Matrix A,                       // matrix to slice
    int ntasks                          // # of tasks
) ;

void GB_ek_slice_merge1     // merge column counts for the matrix C
(
    // input/output:
    int64_t *restrict Cp,                    // column counts
    // input:
    const int64_t *restrict Wfirst,          // size ntasks
    const int64_t *restrict Wlast,           // size ntasks
    const int64_t *ek_slicing,                  // size 3*ntasks+1
    const int ntasks                            // # of tasks
) ;

void GB_ek_slice_merge2     // merge final results for matrix C
(
    // output
    int64_t *C_nvec_nonempty,           // # of non-empty vectors in C
    int64_t *restrict Cp_kfirst,     // size ntasks
    // input/output
    int64_t *restrict Cp,            // size cnvec+1
    // input
    const int64_t cnvec,
    const int64_t *restrict Wfirst,          // size ntasks
    const int64_t *restrict Wlast,           // size ntasks
    const int64_t *ek_slicing,                  // size 3*ntasks+1
    const int ntasks,                   // # of tasks used to construct C
    const int nthreads,                 // # of threads to use
    GB_Context Context
) ;

//------------------------------------------------------------------------------
// GB_get_pA_and_pC: find the part of A(:,k) and C(:,k) for this task
//------------------------------------------------------------------------------

// The tasks were generated by GB_ek_slice.

static inline void GB_get_pA_and_pC
(
    // output
    int64_t *pA_start,
    int64_t *pA_end,
    int64_t *pC,
    // input
    int tid,            // task id
    int64_t k,          // current vector
    int64_t kfirst,     // first vector for this slice
    int64_t klast,      // last vector for this slice
    const int64_t *restrict pstart_slice,   // start of each slice in A
    const int64_t *restrict Cp_kfirst,      // start of each slice in C
    const int64_t *restrict Cp,             // vector pointers for C
    int64_t cvlen,                             // C->vlen
    const int64_t *restrict Ap,             // vector pointers for A
    int64_t avlen                              // A->vlen
)
{

    int64_t p0 = GBP (Ap, k, avlen) ;
    int64_t p1 = GBP (Ap, k+1, avlen) ;

    if (k == kfirst)
    { 
        // First vector for task tid; may only be partially owned.
        (*pA_start) = pstart_slice [tid] ;
        (*pA_end  ) = GB_IMIN (p1, pstart_slice [tid+1]) ;
        (*pC) = Cp_kfirst [tid] ;
    }
    else if (k == klast)
    { 
        // Last vector for task tid; may only be partially owned.
        (*pA_start) = p0 ;
        (*pA_end  ) = pstart_slice [tid+1] ;
        (*pC) = GBP (Cp, k, cvlen) ;
    }
    else
    { 
        // task tid entirely owns this vector A(:,k).
        (*pA_start) = p0 ;
        (*pA_end  ) = p1 ;
        (*pC) = GBP (Cp, k, cvlen) ;
    }
}

//------------------------------------------------------------------------------
// GB_get_pA: find the part of A(:,k) to be operated on by this task
//------------------------------------------------------------------------------

// The tasks were generated by GB_ek_slice.

static inline void GB_get_pA
(
    // output
    int64_t *pA_start,
    int64_t *pA_end,
    // input
    int tid,            // task id
    int64_t k,          // current vector
    int64_t kfirst,     // first vector for this slice
    int64_t klast,      // last vector for this slice
    const int64_t *restrict pstart_slice,   // start of each slice in A
    const int64_t *restrict Ap,             // vector pointers for A
    int64_t avlen                              // A->vlen
)
{

    int64_t p0 = GBP (Ap, k, avlen) ;
    int64_t p1 = GBP (Ap, k+1, avlen) ;

    if (k == kfirst)
    { 
        // First vector for task tid; may only be partially owned.
        (*pA_start) = pstart_slice [tid] ;
        (*pA_end  ) = GB_IMIN (p1, pstart_slice [tid+1]) ;
    }
    else if (k == klast)
    { 
        // Last vector for task tid; may only be partially owned.
        (*pA_start) = p0 ;
        (*pA_end  ) = pstart_slice [tid+1] ;
    }
    else
    { 
        // task tid entirely owns this vector A(:,k).
        (*pA_start) = p0 ;
        (*pA_end  ) = p1 ;
    }
}

#endif

