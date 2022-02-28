//------------------------------------------------------------------------------
// GB_nnz.h: number of entries in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_NNZ_H
#define GB_NNZ_H

#ifdef GB_CUDA_KERNEL

    //--------------------------------------------------------------------------
    // create static inline device functions for the GPU
    //--------------------------------------------------------------------------

    #include "GB_int64_multiply.c"
    #include "GB_nnz_full_template.c"
    #include "GB_nnz_held_template.c"
    #include "GB_nnz_max_template.c"
    #include "GB_nnz_template.c"

#else

    //--------------------------------------------------------------------------
    // declare the regular functions for the CPU
    //--------------------------------------------------------------------------

    // GB_nnz(A): # of entries in any matrix: includes zombies for hypersparse
    // and sparse, but excluding entries flagged as not present in a bitmap.
    GB_PUBLIC int64_t GB_nnz (GrB_Matrix A) ;

    // GB_nnz_full(A): # of entries in A if A is full
    GB_PUBLIC int64_t GB_nnz_full (GrB_Matrix A) ;

    // GB_nnz_held(A): # of entries held in the data structure, including
    // zombies and all entries in a bitmap.  For hypersparse, sparse, and full,
    // GB_nnz(A) and GB_nnz_held(A) are the same.  For bitmap, GB_nnz_held(A)
    // is the same as the # of entries in a full matrix (# rows times #
    // columns).
    GB_PUBLIC int64_t GB_nnz_held (GrB_Matrix A) ;

    // GB_nnz_max(A): max number of entries that can be held in a matrix.
    // For iso full matrices, GB_nnz_max(A) can be less than GB_nnz_full(A),
    // and is typically 1.
    GB_PUBLIC int64_t GB_nnz_max (GrB_Matrix A) ;

#endif

// Upper bound on nnz(A) when the matrix has zombies and pending tuples;
// does not need GB_MATRIX_WAIT(A) first.
#define GB_NNZ_UPPER_BOUND(A) \
    (GB_nnz ((GrB_Matrix) A) - (A)->nzombies + GB_Pending_n (A))

#endif
