//------------------------------------------------------------------------------
// GB_cuda_warmup.cu: warmup the GPU
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0
// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_cuda.h"
/*
#include "rmm/include/rmm/mr/device/managed_memory_resource.hpp"
#include "rmm/include/rmm/mr/device/pool_memory_resource.hpp"
#include "rmm/include/rmm/mr/device/owning_wrapper.hpp"
#include "rmm/include/rmm/mr/device/default_memory_resource.hpp"
#include "rmm/include/rmm/mr/device/per_device_resource.hpp"
#include "rmm/include/rmm/mr/device/cnmem_managed_memory_resource.hpp"
*/
#include "rmm/detail/cnmem.h"

bool GB_cuda_warmup (int device)
{
    // allocate 'nothing' just to load the drivers.
    // No need to free the result.
    double gpu_memory_size = GB_Global_gpu_memorysize_get (device);

    printf ("warming up device %d memsize %g sms %d\n",
        device,
        gpu_memory_size, 
        GB_Global_gpu_sm_get (device)) ;


    //auto cuda_managed = std::make_shared<rmm::mr::managed_memory_resource>();
    //auto cuda = std::make_shared<rmm::mr::cuda_memory_resource>();
    //auto pool = rmm::mr::make_owning_wrapper<rmm::mr::pool_memory_resource>
    //            ( cuda_managed, gpu_memory_size/2, gpu_memory_size ) ;  

    std::vector<int> dev{0};
    cnmemDevice_t cnmem_device;
    memset(&cnmem_device, 0, sizeof(cnmem_device) ) ;
    cnmem_device.size = gpu_memory_size/2;
    if( device ==0)
    {
      cnmemInit(1, &cnmem_device, CNMEM_FLAGS_MANAGED);
    }

    //auto pool = std::make_shared<rmm::mr::cnmem_managed_memory_resource> ( gpu_memory_size/2 ) ;


    //rmm::mr::set_per_device_resource ( rmm::cuda_device_id{device}, 
    //                                 ( rmm::mr::device_memory_resource *)pool.get() ) ;
    
    //rmm::mr::set_default_resource ( pool.get() );
    //rmm::mr::set_current_device_resource ( pool.get() );

    //GB_Global_gpu_device_memory_resource_set( device, (void *)rmm::mr::get_current_device_resource() );

    void *p ;
    //cudaError_t err = cudaMalloc (&p, (size_t) 0) ;
    //p = rmm::mr::get_current_device_resource()->allocate(  256) ;
    //p = pool->allocate( 10) ;
    cnmemMalloc( &p,  256 , NULL);
    //rmm::mr::get_current_device_resource()->deallocate(p, 1);
    //pool->deallocate( p, 10);
    cnmemFree( p, NULL);

    printf ("GPU %d nice and toasty now, pool=%g\n", device, gpu_memory_size/2 ) ;

    // TODO check for jit cache? or in GB_init?

    return  true; //(err == cudaSuccess) ;
}

