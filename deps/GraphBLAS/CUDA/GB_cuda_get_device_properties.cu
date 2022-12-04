//------------------------------------------------------------------------------
// GB_cuda_get_device_properties.cu: get the properties of a GPU
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_cuda.h"

//------------------------------------------------------------------------------
// GB_cuda_get_device: get the current GPU
//------------------------------------------------------------------------------

bool GB_cuda_get_device (int &device)
{
    if (&device == NULL)
    {
        // invalid inputs
        return (false) ;
    }
    CHECK_CUDA_SIMPLE (cudaGetDevice (&device)) ;
    return (true) ;
}

//------------------------------------------------------------------------------
// GB_cuda_set_device: set the current GPU
//------------------------------------------------------------------------------

bool GB_cuda_set_device (int device)
{
    if (device < 0)
    {
        // invalid inputs
        return (false) ;
    }
    CHECK_CUDA_SIMPLE (cudaSetDevice (device)) ;
    return (true) ;
}

//------------------------------------------------------------------------------
// GB_cuda_get_device_properties: determine all properties of a single GPU
//------------------------------------------------------------------------------

bool GB_cuda_get_device_properties  // true if OK, false if failure
(
    int device,
    GB_cuda_device *prop
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (prop == NULL || device < 0)
    {
        // invalid inputs
        return (false) ;
    }

    // clear the GPU settings
    memset (prop, 0, sizeof (GB_cuda_device)) ;

    int old_device ;
    CHECK_CUDA_SIMPLE ( cudaGetDevice( &old_device ) ) ;

    //--------------------------------------------------------------------------
    // get the properties
    //--------------------------------------------------------------------------

    int num_sms, compute_capability_major, compute_capability_minor ;
    size_t memfree, memtotal ;

    CHECK_CUDA_SIMPLE( cudaDeviceGetAttribute (&num_sms,
                                         cudaDevAttrMultiProcessorCount,
                                         device) ) ;
    CHECK_CUDA_SIMPLE( cudaDeviceGetAttribute (&compute_capability_major,
                                         cudaDevAttrComputeCapabilityMajor,
                                         device) ) ;
    CHECK_CUDA_SIMPLE( cudaDeviceGetAttribute (&compute_capability_minor,
                                         cudaDevAttrComputeCapabilityMajor,
                                         device) ) ;

    CHECK_CUDA_SIMPLE ( cudaSetDevice( device ) ) ;
    CHECK_CUDA_SIMPLE ( cudaMemGetInfo( & memfree, &memtotal) ) ;
    CHECK_CUDA_SIMPLE ( cudaSetDevice( old_device ) ) ;

    prop->total_global_memory = memtotal ;
    prop->number_of_sms = num_sms ;
    prop->compute_capability_major = compute_capability_major ;
    prop->compute_capability_minor = compute_capability_minor ;

    printf ("Device: %d: memory: %ld SMs: %d compute: %d.%d\n",
        device, prop->total_global_memory, prop->number_of_sms,
        prop->compute_capability_major, prop->compute_capability_minor) ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (true) ;
}

