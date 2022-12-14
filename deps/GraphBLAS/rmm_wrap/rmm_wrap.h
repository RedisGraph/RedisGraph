//------------------------------------------------------------------------------
// rmm_wrap/rmm_wrap.h: include file for rmm_wrap
//------------------------------------------------------------------------------

// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// example usage in GraphBLAS:

/*
    GrB_init (mode) ;       // ANSI C11 malloc/calloc/realloc/free, no PMR
    GxB_init (mode, mymalloc, mycalloc, myrealloc, myfree) ;

    GxB_init (mode, mymalloc, NULL, NULL, myfree) ;

    GxB_init (mode, mxMalloc, NULL, NULL, mxFree) ;
    GxB_init (mode, pymalloc, pycalloc, pyrealloc, pyfree) ;
    GxB_init (mode, jl_malloc, jl_calloc, jl_realloc, jl_free) ;
    GxB_init (mode, RedisModule_malloc, RedisModule_calloc,
        RedisModule_realloc, RedisModule_realloc) ;

    // using the RMM functions:
    rmm_wrap_initialize (rmm_wrap_managed, 256 * 1000000L, 256 * 1000000000L) ;
    GxB_init (GxB_NONBLOCKING_GPU, rmm_wrap_malloc, rmm_wrap_calloc,
        rmm_wrap_realloc, rmm_wrap_free) ;
    // ... use GraphBLAS on the GPU
    rmm_wrap_finalize ( ) ;
*/

//------------------------------------------------------------------------------

#ifndef RMM_WRAP_H
#define RMM_WRAP_H

#include <cuda_runtime.h>
#include <stddef.h>
#include <stdio.h>

#define RMM_WRAP_CHECK_CUDA(call)                                         \
  do {                                                                    \
    cudaError_t err = call;                                               \
    if (err != cudaSuccess) {                                             \
      const char* str = cudaGetErrorName( err);                           \
      std::cout << "(CUDA runtime) returned " << str;                     \
      std::cout << " (" << __FILE__ << ":" << __LINE__ << ":" << __func__ \
                << "())" << std::endl;                                    \
    }                                                                     \
  } while (0)


#ifdef __cplusplus
extern "C" {
#endif

// TODO describe the modes
typedef enum
{
    rmm_wrap_host = 0,
    rmm_wrap_host_pinned = 1,
    rmm_wrap_device = 2,
    rmm_wrap_managed = 3
}
RMM_MODE ;

// create an RMM resource
int rmm_wrap_initialize
(
    RMM_MODE mode,
    size_t init_pool_size,
    size_t max_pool_size,
    size_t stream_pool_size
) ;

// destroy an RMM resource
void rmm_wrap_finalize (void) ;

// example usage:
    //  rmm_wrap_initialize (rmm_wrap_managed, INT32_MAX, INT64_MAX) ;
    //  GxB_init (GrB_NONBLOCKING, rmm_wrap_malloc, rmm_wrap_calloc,
    //      rmm_wrap_realloc, rmm_wrap_free) ;
    //  use GraphBLAS ...
    //  GrB_finalize ( ) ;
    //  rmm_wrap_finalize ( ) ;

// The two PMR-based allocate/deallocate signatures (C-style):
void *rmm_wrap_allocate (size_t *size) ;
void  rmm_wrap_deallocate (void *p, size_t size) ;

// The four malloc/calloc/realloc/free signatures:
void *rmm_wrap_malloc (size_t size) ;
void *rmm_wrap_calloc (size_t n, size_t size) ;
void *rmm_wrap_realloc (void *p, size_t newsize) ;
void  rmm_wrap_free (void *p) ;

cudaStream_t get_next_stream_from_pool();
cudaStream_t get_stream_from_pool(size_t stream_id);
cudaStream_t get_main_stream();

#ifdef __cplusplus
}
#endif
#endif

