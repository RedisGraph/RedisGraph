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

#pragma once

#include "GB_jit_launcher.h"
#include "GB_cuda_semiring_factory.hpp"

#include "type_name.hpp"

#define JITIFY_PRINT_INSTANTIATION 1
#define JITIFY_PRINT_SOURCE 1
#define JITIFY_PRINT_LOG 1
#define JITIFY_PRINT_PTX 1
#define JITIFY_PRINT_LINKER_LOG 1
#define JITIFY_PRINT_LAUNCH 1

#include "test/dataFactory.hpp"
#include "test/semiringFactory.hpp"
// #include "GB_cuda.h"


#if __cplusplus >= 201103L

/**
 * This file is responsible for picking all the parameters and what kernel variaiton we will use for a given instance
 * - data types
 * - semiring types
 * - binary ops
 * - monoids
 *
 * Kernel factory says "Here's the actual instance I want you to build with the given parameters"
 */

//Kernel jitifiers
template<typename T> class reduceFactory ;


//AxB_dot3_phase1 kernel launchers
template<  typename T_C, typename T_M, typename T_A, typename T_B> class phase1launchFactory ;

//AxB_dot3_phase3 kernel launchers

template<  typename T_C, typename T_M,
         typename T_A, typename T_B, typename T_xy, typename T_z> class launchFactory ;


const std::vector<std::string> compiler_flags{
   "-std=c++14",
   "-remove-unused-globals",
   "-w",
   "-D__CUDACC_RTC__",
   "-I.",
   "-I..",
// "-I../../Include",
   "-I../../Source",
   "-I../../Source/Template",
   "-I../local_cub/block",
   "-I../templates",
   "-I/usr/local/cuda/include"
};

const std::vector<std::string> header_names ={};

template<typename T>
class reduceFactory
{
  std::string base_name = "GB_jit_reduce";


public:
  reduceFactory() {
  }

  bool jitGridBlockLaunch(int gridsz, int blocksz,
                          T* indata, T* output, unsigned int N,
                          std::string OpName)
  {
      dim3 grid(gridsz);
      dim3 block(blocksz);
      bool result = false;
      T dummy;

      std::cout<<" indata type ="<< GET_TYPE_NAME(dummy)<<std::endl;

      // TODO: Need to implement multiple reduction strategies. For now, just assume SUM

      //      if (OpName == "PLUS") {
//         file_callback = &file_callback_plus;
//      }
//      else if (OpName == "MIN") {
//         file_callback = &file_callback_min;
//      }
//      else if (OpName == "MAX") {
//         file_callback = &file_callback_max;
//      }

      std::string hashable_name = base_name + "_" + OpName;
      std::stringstream string_to_be_jitted ;
      string_to_be_jitted <<
      hashable_name << std::endl << R"(#include ")" << hashable_name << R"(.cu")" << std::endl;

      jit::launcher( hashable_name,
                     string_to_be_jitted.str(),
                     header_names,
                     compiler_flags,
                     file_callback)
                   .set_kernel_inst(base_name,
                                    { GET_TYPE_NAME(dummy) })
                   .configure(grid, block)
                   .launch( indata, output, N);

      checkCudaErrors( cudaDeviceSynchronize() );

      return true;
  }

};

#endif  // C++11

