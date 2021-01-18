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

#include "GB_jit_launcher.h"
#include "../templates/reduceUnrolled.cu.jit"
#include "../templates/sparseDotProduct.cu.jit"
#include "../templates/denseDotProduct.cu.jit"
#include "../templates/GB_jit_AxB_dot3_phase1.cu.jit"
#include "../templates/GB_jit_AxB_dot3_phase2.cu.jit"
#include "../templates/GB_jit_AxB_dot3_phase3_dndn.cu.jit"
#include "../templates/GB_jit_AxB_dot3_phase3_vsvs.cu.jit"
#include "../templates/GB_jit_AxB_dot3_phase3_vssp.cu.jit"
#include "../templates/GB_jit_AxB_dot3_phase3_spdn.cu.jit"
#include "../templates/GB_jit_AxB_dot3_phase3_mp.cu.jit"
#include "../templates/GB_jit_AxB_dot3_phase3_warpix.cu.jit"

#include "type_name.hpp"

#define JITIFY_PRINT_INSTANTIATION 1
#define JITIFY_PRINT_SOURCE 1
#define JITIFY_PRINT_LOG 1
#define JITIFY_PRINT_PTX 1
#define JITIFY_PRINT_LINKER_LOG 1
#define JITIFY_PRINT_LAUNCH 1

#include "dataFactory.hpp"
#include "semiringFactory.hpp"
#include "../GB_cuda.h"


#if __cplusplus >= 201103L


//Kernel jitifiers
template<typename T> class reduceFactory ;
template<typename T1, typename T2, typename T3> class dotFactory ;
template<typename T1, typename T2, typename T3> class spdotFactory ;


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
   "-I../../Include",
   "-I../../Source",
   "-I../../Source/Template",
   "-Ilocal_cub/block",
   "-Itemplates",
   "-I/usr/local/cuda/include"
};

const std::vector<std::string> header_names ={};

template<  typename T_C, typename T_M, typename T_A, typename T_B> 
class phase1launchFactory 
{
  std::string base_name = "GB_jit_";
  std::string kernel_name = "AxB_dot3_phase1";
  std::string template_name = "templates_GB_jit_AxB_dot3_phase1_cu";

public: 

  bool jitGridBlockLaunch(int gridsz, int blocksz, 
                          int64_t *nanobuckets, int64_t *blockBucket, 
                          matrix<T_C> *C, matrix<T_M> *M, matrix<T_A> *A, matrix<T_B> *B) 
     {
      
      bool result = false; 

      T_C dumC;
      T_M dumM;
      T_A dumA;
      T_B dumB;

      dim3 grid(gridsz);
      dim3 block(blocksz);

      std::cout<< kernel_name<<
                      " with types "<<GET_TYPE_NAME(dumC)<<","
                                    <<GET_TYPE_NAME(dumM)<<","
                                    <<GET_TYPE_NAME(dumA)<<","
                                    <<GET_TYPE_NAME(dumB)<<std::endl;

    
      jit::launcher( base_name + kernel_name, 
                     jit_template,
                     header_names, 
                     compiler_flags,
                     file_callback)

                   .set_kernel_inst(  base_name + kernel_name ,
                                    { GET_TYPE_NAME(dumC),
                                      GET_TYPE_NAME(dumM),
                                      GET_TYPE_NAME(dumA),
                                      GET_TYPE_NAME(dumB),
                                      })
                   .configure(grid, block)
                   .launch( nanobuckets, blockBucket, C, M, A, B);

      checkCudaErrors( cudaDeviceSynchronize() );
      result= true;

      return result;
     }


};

template<  typename T_C> 
class phase2launchFactory 
{
  std::string base_name = "GB_jit_";
  std::string kernel_name = "AxB_dot3_phase2";
  std::string template_name = "templates_GB_jit_AxB_dot3_phase2_cu";

public: 

  bool jitGridBlockLaunch(int gridsz, int blocksz, 
                          int64_t *nanobuckets, int64_t *blockBucket, 
                          int64_t *bucketp, int64_t *bucket,
                          matrix<T_C> *C, const int64_t cnz, const int64_t nblocks )
     {
      
      bool result = false; 

      T_C dumC;

      dim3 grid(gridsz);
      dim3 block(blocksz);

      std::cout<< kernel_name<<" with types " <<GET_TYPE_NAME(dumC)<<std::endl;

    
      jit::launcher( base_name + kernel_name, 
                     jit_template,
                     header_names, 
                     compiler_flags,
                     file_callback)

                   .set_kernel_inst(  base_name + kernel_name ,
                                    { GET_TYPE_NAME(dumC) })
                   .configure(grid, block)
                   .launch( nanobuckets, blockBucket, bucketp, bucket, C, cnz);

      checkCudaErrors( cudaDeviceSynchronize() );
      result= true;

      return result;
     }

};

template<  typename T_C> 
class phase2endlaunchFactory 
{
  std::string base_name = "GB_jit_";
  std::string kernel_name = "AxB_dot3_phase2end";
  std::string template_name = "templates_GB_jit_AxB_dot3_phase2_cu";

public: 

  bool jitGridBlockLaunch(int gridsz, int blocksz, 
                          int64_t *nanobuckets, int64_t *blockBucket, 
                          int64_t *bucketp, int64_t *bucket,
                          matrix<T_C> *C, const int64_t cnz) 
     {
      
      bool result = false; 

      jit_template = templates_GB_jit_AxB_dot3_phase2_cu;   

      T_C dumC;

      dim3 grid(gridsz);
      dim3 block(blocksz);

      std::cout<< kernel_name<<" with types " <<GET_TYPE_NAME(dumC)<<std::endl;

    
      jit::launcher( base_name + kernel_name, 
                     jit_template,
                     header_names, 
                     compiler_flags,
                     file_callback)

                   .set_kernel_inst(  base_name + kernel_name ,
                                    { GET_TYPE_NAME(dumC) })
                   .configure(grid, block)
                   .launch( nanobuckets, blockBucket, bucketp, bucket, C, cnz);

      checkCudaErrors( cudaDeviceSynchronize() );
      result= true;

      return result;
     }

};

template<  typename T_C, typename T_M, typename T_A, typename T_B, typename T_XY, typename T_Z> 
class launchFactory 
{
  std::string base_name = "GB_jit_";
  std::string kernel_name = "AxB_dot3_phase3_";
  std::string template_name = "___templates_GB_jit_AxB_dot3_phase3_";
  std::string OpName;
  std::string SR;

  GB_callback callback_generator;

public: 
  launchFactory(std::string SemiRing, std::string Optype) {
      if (SemiRing == "PLUS_TIMES") {
         std::cout<<"loading PLUS_TIMES semi-ring"<<std::endl;
         file_callback = semiring_plus_times_callback;
         //callback_generator.load_string( "mySemiring.h", semiring_string);
         //file_callback = callback_generator.callback;
      }
      else if (SemiRing == "MIN_PLUS") {
         std::cout<<"loading MIN_PLUS semi-ring"<<std::endl;
         file_callback = semiring_min_plus_callback;
      }
      else if (SemiRing == "MAX_PLUS") {
         std::cout<<"loading MAX_PLUS semi-ring"<<std::endl;
         file_callback = semiring_max_plus_callback;
      }
      OpName = Optype;
      SR = SemiRing;

  }

  bool jitGridBlockLaunch(int gridsz, int blocksz, 
                          int64_t start, int64_t end, int64_t *Bucket, 
                          matrix<T_C> *C, matrix<T_M> *M, matrix<T_A> *A, matrix<T_B> *B,
                          int sz) 
     {
      
      bool result = false; 

      T_C dumC;
      T_M dumM;
      T_A dumA;
      T_B dumB;
      T_XY dumXY;
      T_Z dumZ;

      dim3 grid(gridsz);
      dim3 block(blocksz);

      std::cout<< kernel_name<<SR<<OpName<<
                      " with types "<<GET_TYPE_NAME(dumC)<<","
                                    <<GET_TYPE_NAME(dumM)<<","
                                    <<GET_TYPE_NAME(dumA)<<","
                                    <<GET_TYPE_NAME(dumB)<<","
                                    <<GET_TYPE_NAME(dumXY)<<","
                                    <<GET_TYPE_NAME(dumZ)<<std::endl;

      const char*  jit_template;
      if (OpName == "dndn") {
         jit_template = ___templates_GB_jit_AxB_dot3_phase3_dndn_cu;   
      }
      if (OpName == "vsvs") {
         jit_template = ___templates_GB_jit_AxB_dot3_phase3_vsvs_cu;   
      }
      if (OpName == "vssp") {
         jit_template = ___templates_GB_jit_AxB_dot3_phase3_vssp_cu;   
      }
      if (OpName == "spdn") {
         jit_template = ___templates_GB_jit_AxB_dot3_phase3_spdn_cu;   
      }
      if (OpName == "mp") {
         jit_template = ___templates_GB_jit_AxB_dot3_phase3_mp_cu;   
      }
      if (OpName == "warp") {
         jit_template = ___templates_GB_jit_AxB_dot3_phase3_warpix_cu;   
      }

      jit::launcher( base_name + SR + OpName+ GET_TYPE_NAME(dumZ), 
                     jit_template,
                     header_names, 
                     compiler_flags,
                     file_callback)

                   .set_kernel_inst(  kernel_name+OpName,
                                    { GET_TYPE_NAME(dumC),
                                      GET_TYPE_NAME(dumA),
                                      GET_TYPE_NAME(dumB),
                                      GET_TYPE_NAME(dumXY),
                                      GET_TYPE_NAME(dumXY),
                                      GET_TYPE_NAME(dumZ)
                                      })
                   .configure(grid, block)
                   .launch( start, end, Bucket,
                            C, M, A, B, sz);

      checkCudaErrors( cudaDeviceSynchronize() );
      result= true;

      return result;
     }

};

template<typename T1, typename T2, typename T3>
class spdotFactory 
{
  std::string base_name = "GBjit_spDot_";
public: 
  spdotFactory() {
  }

  bool jitGridBlockLaunch(int gridsz, int blocksz, unsigned int xn, unsigned int *xi, T1* x,
                                                   unsigned int yn, unsigned int *yi, T2* y, 
                                                        T3* output, std::string OpName) 
  {
      
      bool result = false; 
      if (OpName == "PLUS_TIMES") {
         file_callback = &semiring_plus_times_callback;
      }
      else if (OpName == "MIN_PLUS") {
         file_callback = &semiring_min_plus_callback;
      }

      T1 dum1;
      T2 dum2;
      T3 dum3;

      dim3 grid(gridsz);
      dim3 block(blocksz);

      jit::launcher( base_name + OpName, 
                     ___templates_sparseDotProduct_cu,
                     header_names, 
                     compiler_flags,
                     file_callback)

                   .set_kernel_inst("sparseDotProduct",
                                    { GET_TYPE_NAME(dum1),
                                      GET_TYPE_NAME(dum2),
                                      GET_TYPE_NAME(dum3)})
                   .configure(grid, block)
                   .launch(xn, xi, x, yn, yi, y, output);

      
      checkCudaErrors( cudaDeviceSynchronize() );
      result= true;

      return result;
  }

};

template<typename T1, typename T2, typename T3>
class dotFactory 
{
  std::string base_name = "GBjit_dnDot_";
public: 
  dotFactory() {
  }


  bool jitGridBlockLaunch(int gridsz, int blocksz, T1* x, T2* y, T3* output, unsigned int N, std::string OpName) 
  {
      
      bool result = false; 
      if (OpName == "PLUS_TIMES") {
         file_callback = &semiring_plus_times_callback;
      }
      else if (OpName == "MIN_PLUS") {
         file_callback = &semiring_min_plus_callback;
      }

      T1 dum1;
      T2 dum2;
      T3 dum3;

      dim3 grid(gridsz);
      dim3 block(blocksz);

      jit::launcher( base_name + OpName, 
                     ___templates_denseDotProduct_cu,
                     header_names, 
                     compiler_flags,
                     file_callback)

                   .set_kernel_inst("denseDotProduct",
                                    { GET_TYPE_NAME(dum1),
                                      GET_TYPE_NAME(dum2),
                                      GET_TYPE_NAME(dum3)})
                   .configure(grid, block)
                   .launch(x, y, output, N);

      checkCudaErrors( cudaDeviceSynchronize() );
      result= true;

      return result;
  }

};

template<typename T>
class reduceFactory 
{
  std::string base_name = "GBjit_reduce_";

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
      
      if (OpName == "PLUS") {
         file_callback = &file_callback_plus;
      }
      else if (OpName == "MIN") {
         file_callback = &file_callback_min;
      }   
      else if (OpName == "MAX") {
         file_callback = &file_callback_max;
      }
       

      jit::launcher( base_name + OpName, 
                     ___templates_reduceUnrolled_cu,
                     header_names, 
                     compiler_flags,
                     file_callback)
                   .set_kernel_inst("reduceUnrolled",
                                    { GET_TYPE_NAME(dummy) })
                   .configure(grid, block)
                   .launch( indata, output, N);
      
      checkCudaErrors( cudaDeviceSynchronize() );

      result= true;

     
      return result;
  }

};

#endif  // C++11

