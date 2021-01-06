//------------------------------------------------------------------------------
// GxB_cuda_malloc.c: wrapper for cudaMallocManaged, or just malloc if no CUDA
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// DRAFT: in progress

#include "GB.h"

void *GxB_cuda_malloc (size_t size)         // standard malloc signature
{
    #if defined ( GBCUDA )
    return (GB_cuda_malloc (size)) ;
    #else
    return (malloc (size)) ;
    #endif
}

