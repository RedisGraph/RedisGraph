//------------------------------------------------------------------------------
// GB_cuda_gateway.h: definitions for interface to GB_cuda_* functions
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// CUDA gateway functions (DRAFT: in progress)

// This file can be #include'd into any GraphBLAS/Source file that needs to
// call a CUDA gateway function, or use the typedef defined below.  It is also
// #include'd in GraphBLAS/CUDA/GB_cuda.h, for use by the CUDA/GB_cuda_*.cu
// gateway functions.

// If GBCUDA is defined in GraphBLAS/CMakeLists.txt, then GraphBLAS can call
// the C-callable gateway functions defined in GraphBLAS/CUDA/*.cu source
// files.  If GBCUDA is not defined, then these functions are not called.  The
// typedef always appears, since it is part of the GB_Global struct, whether
// or not CUDA is used.

#ifndef GB_CUDA_GATEWAY_H
#define GB_CUDA_GATEWAY_H

#define GB_CUDA_MAX_GPUS 32

// The GPU is only used if the work is larger than the GxB_GPU_CHUNK.
// The default value of this parameter is GB_GPU_CHUNK_DEFAULT:
#define GB_GPU_CHUNK_DEFAULT (1024*1024)

#if defined ( GB_NVCC )
extern "C" {
#endif

//------------------------------------------------------------------------------
// GB_cuda_device: properties of each GPU in the system
//------------------------------------------------------------------------------

typedef struct
{
    char    name [256] ;
    size_t  total_global_memory ;
    int  number_of_sms ;
    int  compute_capability_major;
    int  compute_capability_minor;
    bool use_memory_pool;
    int  pool_size;             // TODO: should this be size_t?
    int  max_pool_size;         // TODO: should this be size_t?
    void *memory_resource;
}
GB_cuda_device ;

//------------------------------------------------------------------------------
// GB_ngpus_to_use: determine # of GPUs to use for the next computation
//------------------------------------------------------------------------------

static inline int GB_ngpus_to_use
(
    double work                 // total work to do
)
{
    // get the current GxB_GPU_CONTROL setting
    GrB_Desc_Value gpu_control = GB_Global_gpu_control_get ( ) ;
    int gpu_count = GB_Global_gpu_count_get ( ) ;
    if (gpu_control == GxB_GPU_NEVER || gpu_count == 0)
    {
        // never use the GPU(s)
        return (0) ;
    }
    else if (gpu_control == GxB_GPU_ALWAYS)
    {
        // always use all available GPU(s)
        return (gpu_count) ;
    }
    else
    {
        // use no more than max_gpus_to_use
        double gpu_chunk = GB_Global_gpu_chunk_get ( ) ;
        double max_gpus_to_use = floor (work / gpu_chunk) ;
        // but use no more than the # of GPUs available
        if (max_gpus_to_use > gpu_count) return (gpu_count) ;
        return ((int) max_gpus_to_use) ;
    }
}


//------------------------------------------------------------------------------
// GB_cuda_* gateway functions
//------------------------------------------------------------------------------

bool GB_cuda_get_device_count   // true if OK, false if failure
(
    int *gpu_count              // return # of GPUs in the system
) ;

bool GB_cuda_warmup (int device) ;

bool GB_cuda_get_device( int *device) ;

bool GB_cuda_set_device( int device) ;

bool GB_cuda_get_device_properties
(
    int device,
    GB_cuda_device *prop
) ;

// There is no GB_cuda_realloc function, since CUDA does not have a
// realloc function.
void *GB_cuda_malloc (size_t size) ;           // standard malloc signature
void  GB_cuda_free (void *p) ;                 // standard free signature
void *GB_cuda_calloc (size_t n, size_t size) ; // standard calloc signature

GrB_Info GB_cuda_red__plus_int64
(
    int64_t *result,
    int64_t *Ax,
    int64_t anz,
    int64_t *restrict W,      // array of size ntasks
    int64_t worksize,
    int ntasks,
    int nthreads,
    int blocksize
) ;

GrB_Info GB_AxB_dot3_cuda           // C<M> = A'*B using dot product method
(
    GrB_Matrix C,                   // output matrix, static header
    const GrB_Matrix M,             // mask matrix
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

#if defined ( GB_NVCC )
}
#endif

#endif

