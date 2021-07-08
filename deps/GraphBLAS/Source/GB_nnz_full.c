//------------------------------------------------------------------------------
// GB_nnz_full.c: number of entries in a full matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

int64_t GB_nnz_full      // return nnz(A) or INT64_MAX if integer overflow
(
    GrB_Matrix A
)
{ 
    GrB_Index anz ;
    bool ok = GB_Index_multiply (&anz, A->vlen, A->vdim) ;
    return (ok ? ((int64_t) anz) : INT64_MAX) ;
}

