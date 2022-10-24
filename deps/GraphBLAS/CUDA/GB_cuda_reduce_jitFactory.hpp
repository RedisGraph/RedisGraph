// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (c) 2017-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of NVIDIA CORPORATION nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
  Extended example for building on-the-fly kernels with C interface.
  Simple examples demonstrating different ways to load source code
    and call kernels.
 */

#ifndef GB_REDUCE_JITFACTORY_H
#define GB_REDUCE_JITFACTORY_H

#pragma once
#include "GB_cuda_reduce_factory.hpp"

/**
 * This file is responsible for picking all the parameters and what kernel variaiton we will use for a given instance
 * - data types
 * - semiring types
 * - binary ops
 * - monoids
 *
 * Kernel factory says "Here's the actual instance I want you to build with the given parameters"
 */

//bool GB_cuda_reduce(int64_t *index, void *in_data, void *output, unsigned int N, GrB_Monoid op);

//Kernel jitifiers
class reduceFactory ;
template<typename T1, typename T2, typename T3> class spdotFactory ;

//------------------------------------------------------------------------------
// reduceFactory
//------------------------------------------------------------------------------

class reduceFactory
{
  std::string base_name = "GB_jit";
  std::string kernel_name = "reduceNonZombiesWarp";

  int threads_per_block = 320 ;
  int work_per_thread = 256;
  int number_of_sms = GB_Global_gpu_sm_get (0);

  GB_cuda_reduce_factory &reduce_factory_;

public:

  reduceFactory(GB_cuda_reduce_factory &myreducefactory) : reduce_factory_(myreducefactory) {}

  int get_threads_per_block() {
    return threads_per_block;
  }

  int get_number_of_blocks(unsigned int N) {
      return (N + work_per_thread*threads_per_block - 1)/(work_per_thread*threads_per_block);
  }

  // Note: this does assume the erased types are compatible w/ the monoid's ztype
  bool jitGridBlockLaunch(GrB_Matrix A, void* output,
                          GrB_Monoid op, cudaStream_t stream = 0)
  {
      GBURBLE ("\n(launch reduce factory) \n") ;

      GrB_Scalar temp_scalar;
      GrB_Scalar_new(&temp_scalar, op->op->ztype);

      cuda::jit::scalar_set_element(temp_scalar, 0);
      GrB_Scalar_wait(temp_scalar, GrB_MATERIALIZE);

      jit::GBJitCache filecache = jit::GBJitCache::Instance() ;
      filecache.getFile (reduce_factory_) ;

      auto rcode = std::to_string(reduce_factory_.rcode);

      std::string hashable_name = base_name + "_" + kernel_name;
      std::stringstream string_to_be_jitted ;
      string_to_be_jitted <<
      hashable_name << std::endl <<
      R"(#include ")" << jit::get_user_home_cache_dir() << "/" << reduce_factory_.filename << R"(")" << std::endl <<
      R"(#include ")" << hashable_name << R"(.cuh")" << std::endl;

      bool is_sparse = GB_IS_SPARSE(A);
      int64_t N = is_sparse ? GB_nnz(A) : GB_NCOLS(A) * GB_NROWS(A);

      int blocksz = get_threads_per_block();
      int gridsz = get_number_of_blocks(N);
      dim3 grid(gridsz);
      dim3 block(blocksz);

      // FIXME: call GB_stringify_reduce to create GB_ADD and related
      // macros, in an include file: GB_reduce_123412341234.h
      GBURBLE ("(cuda reduce launch %d threads in %d blocks)", blocksz, gridsz ) ;

      jit::launcher(hashable_name + "_" + rcode,
                    string_to_be_jitted.str(),
                    header_names,
                    compiler_flags,
                    file_callback)
               .set_kernel_inst(  kernel_name , { A->type->name, op->op->ztype->name, rcode, "true" })
               .configure(grid, block, SMEM, stream)
               // FIXME: GB_ADD is hardcoded into kernel for now
               .launch( A, temp_scalar, N, is_sparse);

      // Need to synchronize before copying result to host
      CHECK_CUDA( cudaStreamSynchronize(stream) );

      memcpy(output, temp_scalar->x, op->op->ztype->size);

      rmm_wrap_free(temp_scalar);
      return true;
  }
};

//------------------------------------------------------------------------------

inline bool GB_cuda_reduce(GB_cuda_reduce_factory &myreducefactory,
                           GrB_Matrix A, void *output, GrB_Monoid op,
                           cudaStream_t stream = 0) {
    reduceFactory rf(myreducefactory);
    GBURBLE ("(starting cuda reduce)" ) ;
    bool result = rf.jitGridBlockLaunch(A, output, op, stream);
    GBURBLE ("(ending cuda reduce)" ) ;
    return (result) ;
}

#endif
