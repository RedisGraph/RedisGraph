//------------------------------------------------------------------------------
// GB_nnz.h: macros for matrices and vectors for counting # of entries
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_NNZ_H
#define GB_NNZ_H

// If A->nzmax is zero, then A->p might not be allocated.  Note the GB_NNZ
// macro does not count pending tuples; use GB_MATRIX_WAIT(A) first, if
// needed.  For both sparse and hypersparse matrices Ap [0] == 0,
// and nnz(A) = Ap [nvec].  For full matrices, Ap is NULL.  For bitmap
// matrices, GB_NNZ (A) is given by A->nvals, but the size of the space
// is GB_NNZ_HELD (A).

// nnz(A) if A is sparse or hypersparse
#define GB_NNZ_SPARSE(A) ((A)->p [(A)->nvec])

// nnz(A) if A is full
#define GB_NNZ_FULL(A) ((A)->vlen * (A)->vdim)

// nnz(A) if A is bitmap
#define GB_NNZ_BITMAP(A) ((A)->nvals)

// nnz(A) if A is full or bitmap
#define GB_NNZ_FULL_OR_BITMAP(A) \
    (((A)->b == NULL) ? GB_NNZ_FULL (A) : GB_NNZ_BITMAP (A))

// nnz(A) for all non-empty matrices
#define GB_NNZ_NONEMPTY(A) \
    (((A)->p == NULL) ? GB_NNZ_FULL_OR_BITMAP (A) : GB_NNZ_SPARSE (A))

// nnz(A) for any matrix: includes zombies for hypersparse and sparse,
// but excluding entries flagged as not present in a bitmap.
#define GB_NNZ(A) (((A)->nzmax <= 0) ? 0 : GB_NNZ_NONEMPTY (A))

// nnz_held(A) is the number of entries held in the data structure, including
// zombies and all entries in a bitmap.  For hypersparse, sparse, and full,
// nnz(A) and nnz_held(A) are the same.  For bitmap, nnz_held(A) is the
// same as the # of entries in a full matrix (# rows times # columns).
#define GB_NNZ_HELD(A) (((A)->nzmax <= 0) ? 0 : GB_NNZ_HELD_NONEMPTY (A))

// nnz_held(A) for all non-empty matrices
#define GB_NNZ_HELD_NONEMPTY(A) \
    (((A)->p == NULL) ? GB_NNZ_FULL (A) : GB_NNZ_SPARSE (A))

// Upper bound on nnz(A) when the matrix has zombies and pending tuples;
// does not need GB_MATRIX_WAIT(A) first.
#define GB_NNZ_UPPER_BOUND(A) ((GB_NNZ (A) - (A)->nzombies) + GB_Pending_n (A))

#endif

