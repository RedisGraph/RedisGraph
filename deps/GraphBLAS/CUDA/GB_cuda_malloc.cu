//------------------------------------------------------------------------------
// GB_cuda_malloc.cu: wrapper for cuda Managed Memory allocator, or pool 
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_cuda.h"
#include "rmm/detail/cnmem.h"

void *GB_cuda_malloc (size_t size)          // standard malloc signature
{
    void *p = NULL ;

    cnmemMalloc( &p, size, NULL);

    return p;
  
     
}

