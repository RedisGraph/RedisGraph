//------------------------------------------------------------------------------
// GB_nnz.c: number of entries in a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

// GB_nnz (A) for any matrix: includes zombies for hypersparse and sparse,
// but excluding entries flagged as not present in a bitmap.  Pending tuples
// are ignored; to count them, use GB_wait first.  A may be NULL.

int64_t GB_nnz      // return nnz(A) or INT64_MAX if integer overflow
(
    GrB_Matrix A
)
{

    if (A == NULL || A->magic != GB_MAGIC || A->x == 0)
    { 
        // A is NULL or uninitialized
        return (0) ;
    }
    else if (A->p != NULL)
    { 
        // A is sparse or hypersparse
        return (A->p [A->nvec]) ;
    }
    else if (A->b != NULL)
    { 
        // A is bitmap
        return (A->nvals) ;
    }
    else
    { 
        // A is full
        return (GB_nnz_full (A)) ;
    }
}

