//------------------------------------------------------------------------------
// GB_cuda_calloc.cu: wrapper for cudaMallocManaged and memset
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_cuda.h"

void *GB_cuda_calloc (size_t n, size_t size)   // standcard calloc signature
{

    // malloc the space
    void *p = GB_cuda_malloc (n * size) ;

    if (p == NULL)
    {
        // out of memory, or other CUDA error
        return (NULL) ;
    }

    // set the space to zero
    memset (p, 0, n * size) ;

    // return the result
    return (p) ;
}

