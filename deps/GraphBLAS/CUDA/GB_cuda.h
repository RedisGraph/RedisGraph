//------------------------------------------------------------------------------
// GB_cuda.h: definitions for using CUDA in GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS/CUDA, (c) NVIDIA Corp. 2017-2019, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This file is #include'd only in the GraphBLAS/CUDA/GB_cuda*.cu source files.

#ifndef GB_CUDA_H
#define GB_CUDA_H

// nvcc chokes on the 'restrict' keyword, so define it to the empty string
// for compiling the *.cu files.
#define restrict

// nvcc also chokes on fpclassify (an ANSI C11 construct that does not appear
// in C++11, it seems).  It also issues spurious warnings about compiler
// pragmas.  Source/GB.h avoids these constructs if GB_NVCC is defined.
#define GB_NVCC

extern "C"
{
    #include "GB.h"
    #include "GB_Global.h"
    #include <stdint.h>
    #include <stddef.h>
}

// GB_cuda_gateway.h is also included in Source/GB* files, which are not
// compiled with nvcc, nor do they see the cuda.h or cuda_runtime.h
// definitions.  Thus, this #include comes first.
#include "GB_cuda_gateway.h"

#include "GB_cuda_global.h"

// Finally, include the CUDA definitions
#include "cuda.h"
#include "cuda_runtime.h"
#include "jitify.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

#define CHECK_CUDA_SIMPLE(call)                                           \
  do {                                                                    \
    cudaError_t err = call;                                               \
    if (err != cudaSuccess) {                                            \
      const char* str = cudaGetErrorName( err);                           \
      std::cout << "(CUDA runtime) returned " << str;                     \
      std::cout << " (" << __FILE__ << ":" << __LINE__ << ":" << __func__ \
                << "())" << std::endl;                                    \
      return (GrB_PANIC) ;                                                \
    }                                                                     \
  } while (0)


//------------------------------------------------------------------------------
// GB_CUDA_CATCH: catch error from a try { ... } region
//------------------------------------------------------------------------------

// Usage:  Must be used in a GB* function that returns GrB_Info, and has a
// GB_Context Context parameter.
//
//  #define GB_FREE_ALL { some macro to free all temporaries }
//  GrB_Info info ;
//  try { ... do stuff that can through an exception }
//  GB_CUDA_CATCH (info) ;

#define GB_CUDA_CATCH(info)                                                    \
    catch (std::exception& e)                                                  \
    {                                                                          \
        printf ("CUDA error: %s\n", e.what ( )) ;                              \
        info = GrB_PANIC ;                                                     \
        /* out_of_memory : info = GrB_OUT_OF_MEMORY ; */                       \
        /* nulltpr:  info = ... ; */                                           \
        /* no gpus here: info = GrB_PANIC ; */                                 \
    }                                                                          \
    if (info != GrB_SUCCESS)                                                   \
    {                                                                          \
        /* CUDA failed */                                                      \
        GB_FREE_ALL ;                                                          \
        return (GB_ERROR (info, (GB_LOG, "CUDA died\n"))) ;                    \
    }

// 12 buckets: computed by up to 11 kernel launches (zombies need no work...),
// using 5 different kernels (with different configurations depending on the
// bucket).
    #include "GB_cuda_buckets.h"
extern "C"
{
    #include "GB_stringify.h"
    
}
#endif

