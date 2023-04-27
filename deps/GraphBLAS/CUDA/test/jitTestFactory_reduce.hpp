// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cassert>
#include <cmath>
#include <random>
#include <algorithm>
#include "../jitFactory.hpp"
//#include "GB_binary_search.h"
#include "GpuTimer.h"
#include "../../rmm_wrap/rmm_wrap.h"
#include <gtest/gtest.h>
#include "dataFactory.hpp"

//reduceNonZombiesWarp kernel
template <typename T>
bool test_reduce_non_zombies_warp_factory( unsigned int, std::string ) ;

//Fixture to generate valid inputs and hold them for tests
class ReduceNonZombiesWarpTest : public ::testing::Test
{
    void SetUp()
    {


    }

    void TearDown()
    {

    }

};

template <typename T>
bool test_reduce_non_zombies_warp_factory( unsigned int N, std::string OP) {

  reduceFactory<T> rF;

  int block(32);
  int nblock= (N + 8*block -1)/(8*block);
  int grid(nblock);
  T* d_data;
  T* output;

  //std::cout<<" alloc'ing data and output"<<std::endl;
  CHECK_CUDA( cudaMallocManaged((void**) &d_data, nblock*sizeof(T)) );
  CHECK_CUDA( cudaMallocManaged((void**) &output, nblock*sizeof(T)) );
  std::cout<<" alloc done"<<std::endl;
  std::cout<<" data fill start"<<std::endl;

  fillvector_linear<T> ( N, d_data);

  std::cout<<" data fill complete"<<std::endl;
  //we will get a triangular sum = N*(N+1)/2 with this input
  //for (unsigned int i =0; i < N; ++i) d_data[i] = i;

  std::cout<< " init data done"<<std::endl;
  //for (unsigned int i =0; i < N; ++i) std::cout<< d_data[i] <<" ";


  T sum;
  std::cout << "Launching reduce"<<OP<<GET_TYPE_NAME(sum)<<" kernel..."<<std::endl;
  rF.jitGridBlockLaunch( grid, block, d_data, output, N, OP );

  for (int i =0; i< nblock; ++i) std::cout<< output[i] <<" ";

  if (OP == "PLUS"){
      sum = (T) 0;
      myOpPTR<T> = myOP_plus<T>;
  }
  if (OP == "MIN") {
      sum = (T)std::numeric_limits<T>::max();
      myOpPTR<T> = myOP_min<T>;
  }
  if (OP == "MAX") {
      sum = (T)std::numeric_limits<T>::min();
      myOpPTR<T> = myOP_max<T>;
  }

  for (int i =0; i< nblock; ++i) sum = (*myOpPTR<T>)(sum ,output[i]);

  T expect;
  bool result = true;
  if (OP == "PLUS") {
     expect  = (T)(N*(N-1)/2);
     T temp = (sum - expect) ;
     if (temp < 0) temp = -temp ;
     //result = (temp < (T)1) ; //adjust formula for leading 0
     EXPECT_LE(temp, 1);
  }
  else if (OP == "MAX") {
     expect = (T)(N-1);
     //result = (sum)== (T)(N-1) ; //max is N-1
     EXPECT_EQ( sum , (T)(N-1) );

  }
  else if (OP == "MIN") {
     expect = (T)0;
     //result = (sum)== (T)(0) ;   //min is 0
     EXPECT_EQ( sum , (T)(0) );
  }
  else expect = (T) 0;
  std::cout <<std::endl<<"result of test_reducefactoryUM with "<< OP<< " operation ="<< sum
            <<" expected "<<expect << std::endl;

  cudaFree(d_data);
  cudaFree(output);
  return result;
}

