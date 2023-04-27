//------------------------------------------------------------------------------
// GB_cuda_get_device_properties.cu: get the properties of a GPU
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_cuda.h"

bool GB_cuda_get_device ( int &device){
    bool goodreturn = false;
    if (&device == NULL)
    {
        // invalid inputs
        return (false) ;
    }

    CHECK_CUDA_SIMPLE ( cudaGetDevice( &device ) ); 
    goodreturn = true;

    return goodreturn;

}

bool GB_cuda_set_device( int device) {
    bool goodreturn = false;
    if (device < 0)
    {
        // invalid inputs
        return (false) ;
    }

    CHECK_CUDA_SIMPLE ( cudaSetDevice( device ) ); 
    goodreturn = true;

    return goodreturn;
}

bool GB_cuda_get_device_properties  // true if OK, false if failure
(
    int device,
    rmm_device *prop
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------
    bool goodreturn = false;
    if (prop == NULL || device < 0)
    {
        // invalid inputs
        return (false) ;
    }

    int old_device;
    CHECK_CUDA_SIMPLE ( cudaGetDevice( &old_device ) ) ; 


    //--------------------------------------------------------------------------
    // get the properties
    //--------------------------------------------------------------------------
    int num_sms;
    int compute_capability_major;
    int compute_capability_minor;
    size_t memfree, memtotal;

    CHECK_CUDA_SIMPLE( cudaDeviceGetAttribute (&num_sms,
                                         cudaDevAttrMultiProcessorCount,
                                         device) );
    CHECK_CUDA_SIMPLE( cudaDeviceGetAttribute (&compute_capability_major,
                                         cudaDevAttrComputeCapabilityMajor,
                                         device) );
    CHECK_CUDA_SIMPLE( cudaDeviceGetAttribute (&compute_capability_minor,
                                         cudaDevAttrComputeCapabilityMajor,
                                         device) );

    CHECK_CUDA_SIMPLE ( cudaSetDevice( device ) ); 
    CHECK_CUDA_SIMPLE ( cudaMemGetInfo( & memfree, &memtotal) ) ;
    CHECK_CUDA_SIMPLE ( cudaSetDevice( old_device ) ); 

    prop->total_global_memory = memtotal;
    prop->number_of_sms = num_sms;
    prop->compute_capability_major = compute_capability_major;
    prop->compute_capability_minor = compute_capability_minor;
    
    goodreturn = true;
    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return  goodreturn;
}

