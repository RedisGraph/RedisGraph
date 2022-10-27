//------------------------------------------------------------------------------
// GB_cuda_init: initialize the GPUs for use by GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_cuda_init queries the system for the # of GPUs available, their memory
// sizes, SM counts, and other capabilities.  Unified Memory support is
// assumed.  Then each GPU is "warmed up" by allocating a small amount of
// memory.

#include "GB.h"

GrB_Info GB_cuda_init (void)
{
    GB_Global_gpu_control_set (GxB_DEFAULT) ;
    if (!GB_Global_gpu_count_set (true)) return (GrB_PANIC) ;
    int gpu_count = GB_Global_gpu_count_get ( ) ;
    for (int device = 0 ; device < 1 ; device++) // TODO for GPU: gpu_count
    {
        // query the GPU and then warm it up
        if (!GB_Global_gpu_device_properties_get (device))
        {
            return (GrB_PANIC) ;
        }
        if (!GB_cuda_warmup (device))
        {
            return (GrB_PANIC) ;
        }
    }
    // make GPU 0 the default device
    GB_cuda_set_device( 0 );
    // also check for jit cache, pre-load library of common kernels ...
    return (GrB_SUCCESS) ;
}

