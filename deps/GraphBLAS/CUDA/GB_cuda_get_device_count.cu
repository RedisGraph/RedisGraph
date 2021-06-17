//------------------------------------------------------------------------------
// GB_cuda_get_device_count.cu: find out how many GPUs exist
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_cuda.h"

bool GB_cuda_get_device_count   // true if OK, false if failure
(
    int *gpu_count              // return # of GPUs in the system
)
{
    cudaError_t err = cudaGetDeviceCount (gpu_count) ;
    return (err == cudaSuccess) ;
}

