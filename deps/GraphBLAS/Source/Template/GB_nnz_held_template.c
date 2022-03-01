//------------------------------------------------------------------------------
// GB_nnz_held_template.c: number of entries held in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_nnz_held(A) is the number of entries held in the data structure, including
// zombies and all entries in a bitmap.  For hypersparse, sparse, and full,
// nnz(A) and nnz_held(A) are the same.  For bitmap, nnz_held(A) is the
// same as the # of entries in a full matrix (# rows times # columns).

#ifdef GB_CUDA_KERNEL
__device__ static inline
#endif
int64_t GB_nnz_held
(
    GrB_Matrix A
)
{

    if (A == NULL || A->magic != GB_MAGIC || A->x == NULL)
    { 
        // A is NULL or not initialized
        return (0) ;
    }
    else if (A->p != NULL)
    { 
        // A is sparse or hypersparse
        return (A->p [A->nvec]) ;
    }
    else
    { 
        // A is bitmap or full
        return (GB_nnz_full (A)) ;
    }
}

