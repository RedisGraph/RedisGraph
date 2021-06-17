//------------------------------------------------------------------------------
// GB_cuda_free.cu: wrapper for cudaFree
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_cuda.h"
#include "rmm/detail/cnmem.h"

void GB_cuda_free (void *p)     // standard free signature
{
    cnmemFree( p , NULL);
    //printf(" GPU %d freeing mem\n", device);
}

