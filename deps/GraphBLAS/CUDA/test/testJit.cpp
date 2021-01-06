/*
 * Copyright (c) 2017-2020, NVIDIA CORPORATION. All rights reserved.
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


#include "jitTestFactory.hpp"


int main(int argc, char* argv[]) {
#if __cplusplus >= 201103L

#define TEST_RESULT(result) (result ? "PASSED" : "FAILED")
std::cout << "Running tests..."<<std::endl;

  std::cout<<" spdot_plus_times_ffd_non_uniform"<<std::endl;
  bool test_spdot_plus_times_ffd_nu = test_spdotfactoryUM<float,float,double>(256, 32,120,"PLUS_TIMES");
  std::cout << "test_spdotfactoryUM<float,float,double> non-uniform uncached:       " 
            << TEST_RESULT(test_spdot_plus_times_ffd_nu)
            << std::endl;

  std::cout<<" spdot_plus_times_ffd_large_non_uniform"<<std::endl;
  bool test_spdot_plus_times_ffd_lrg_nu = test_spdotfactoryUM<float,float,double>(4096, 256,256,"PLUS_TIMES");
  std::cout << "test_spdotfactoryUM<float,float,double> non-uniform uncached:       " 
            << TEST_RESULT(test_spdot_plus_times_ffd_lrg_nu)
            << std::endl;

  std::cout<<" spdot_plus_times_fff"<<std::endl;
  bool test_spdot_plus_times_fff = test_spdotfactoryUM<float,float,float>(256, 32,32,"PLUS_TIMES");
  std::cout << "test_spdotfactoryUM<float,float,float> uncached:       " 
            << TEST_RESULT(test_spdot_plus_times_fff)
            << std::endl;

  std::cout<<" spdot_plus_times_ffd"<<std::endl;
  bool test_spdot_plus_times_ffd = test_spdotfactoryUM<float,float,double>(256, 32,32,"PLUS_TIMES");
  std::cout << "test_spdotfactoryUM<float,float,double> uncached:       " 
            << TEST_RESULT(test_spdot_plus_times_ffd)
            << std::endl;


  cudaSetDevice(0); 
  bool test_spdot_batch_ffff = test_spdot_batch_factoryUM<float, float, float, float>(5, 32, 128, 128, "PLUS_TIMES"); 
  std::cout << "test_spdot_batchUM<float,float,float,float> uncached:       " 
            << TEST_RESULT(test_spdot_batch_ffff)
            << std::endl;

  bool test_spdot_batch_iiii = test_spdot_batch_factoryUM<int, int, int, int>(5, 32, 128, 128, "PLUS_TIMES"); 
  std::cout << "test_spdot_batchUM<int,int,int,int> uncached:       " 
            << TEST_RESULT(test_spdot_batch_iiii)
            << std::endl;

  cudaSetDevice(1); 

  bool test_spdot_batch_liii= test_spdot_batch_factoryUM<int64_t, int, int, int>(5, 32, 256, 128, "PLUS_TIMES"); 
  std::cout << "test_spdot_batchUM<int64_t,int, int,int> uncached:       " 
            << TEST_RESULT(test_spdot_batch_liii)
            << std::endl;
/*



  bool test_dot_min_plus_iil = test_dotfactoryUM<int,int,long>(4096,"MIN_PLUS");
  std::cout << "test_dotfactoryUM<int,int,long> uncached:       " 
            << TEST_RESULT(test_dot_min_plus_iil)
            << std::endl;

  bool test_dot_min_plus_ffd = test_dotfactoryUM<float,float,double>(4096,"MIN_PLUS");
  std::cout << "test_dotfactoryUM<float,float,double> uncached:       " 
            << TEST_RESULT(test_dot_min_plus_ffd)
            << std::endl;

  bool test_dot_plus_times_ffd = test_dotfactoryUM<float,float,double>(4096,"PLUS_TIMES");
  std::cout << "test_dotfactoryUM<float,float,double> uncached:       " 
            << TEST_RESULT(test_dot_plus_times_ffd)
            << std::endl;

  bool test_dot_plus_times_fii = test_dotfactoryUM<float,int,int>(4096,"PLUS_TIMES");
  std::cout << "test_dotfactoryUM<float,int,int> uncached:       " 
            << TEST_RESULT(test_dot_plus_times_fii)
            << std::endl;

  bool test_dot_plus_times_iil = test_dotfactoryUM<int,int,long>(4096,"PLUS_TIMES");
  std::cout << "test_dotfactoryUM<int,int,long> uncached:       " 
            << TEST_RESULT(test_dot_plus_times_iil)
            << std::endl;

  bool test_reducefactory_float_result = test_reducefactoryUM<float>(4096, "PLUS");
  std::cout << "test_reducefactoryUM<float> uncached:       " 
            << TEST_RESULT(test_reducefactory_float_result)
            << std::endl;

  bool test_reducefactory_double_plus_result = test_reducefactoryUM<double>(4096, "PLUS");
  std::cout << "test_reducefactoryUM<double> uncached:       " 
            << TEST_RESULT(test_reducefactory_double_plus_result)
            << std::endl;

  std::cout << "testing cached kernel" <<std::endl;
  bool test2_reducefactory_double_plus_result = test_reducefactoryUM<double>(4096, "PLUS");
  std::cout << "test_reducefactoryUM<double> cached:       " 
            << TEST_RESULT(test2_reducefactory_double_plus_result)
            << std::endl;

  bool test_reducefactory_float_min_result = test_reducefactoryUM<float>(32,"MIN");
  std::cout << "test_reducefactoryUM<float> MIN uncached:       " 
            << TEST_RESULT(test_reducefactory_float_min_result)
            << std::endl;

  bool test_reducefactory_int_min_result = test_reducefactoryUM<int>(32,"MIN");
  std::cout << "test_reducefactoryUM<int> MIN uncached:       " 
            << TEST_RESULT(test_reducefactory_int_min_result)
            << std::endl;

  bool test_reducefactory_int_max_result = test_reducefactoryUM<int>(32,"MAX");
  std::cout << "test_reducefactoryUM<int> MAX uncached:       " 
            << TEST_RESULT(test_reducefactory_int_max_result)
            << std::endl;

  bool test_reducefactory_int_result = test_reducefactoryUM<int>(4096,"PLUS");
  std::cout << "test_reducefactoryUM<int> PLUS uncached:       " 
            << TEST_RESULT(test_reducefactory_int_result)
            << std::endl;

  bool test_reducefactory_int_cache_result = 
                test_reducefactoryUM<int>(4096,"PLUS");
  std::cout << "test_reducefactoryUM<int> PLUS cached:          " 
            << TEST_RESULT(test_reducefactory_int_cache_result)
            << std::endl;
*/
  return 0;

#else
  std::cout << "Tests require building with C++14 support (make CXX14=1)"
            << std::endl;
  return 0;
#endif
}
