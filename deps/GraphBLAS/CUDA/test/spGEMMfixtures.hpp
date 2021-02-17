// SPDX-License-Identifier: Apache-2.0

// SpGEMM Test Fixtures
// Provides test setup and teardown, data generators and covers
// all 12 cases for the masked GEMM ( C, M, A, B) in GraphBLAS
// Connects to the jitFactory for launches.

#include <cassert>
#include <cmath>
#include <random>
#include <algorithm>
#include <cstdint>
#include "jitTestFactory.hpp"
#include "gtest/gtest.h"


//Test generators using jitify


TEST(SpGEMMvsvsTest, PlusTimesLongBoolIntInt) {
  test_spGEMM_vsvs_factory<int64_t, uint8_t, int, int>(5, 32, 256, 128, "PLUS_TIMES"); 
}

TEST(SpGEMMvsvsTest, PlusTimesInt4Test ) {

  test_spGEMM_vsvs_factory<int, int, int, int>(5, 32, 256, 128, "PLUS_TIMES"); 
}

TEST(SpGEMMvsvsTest, PlusTimesInt4TestMed ) {

  test_spGEMM_vsvs_factory<int, int, int, int>(5, 4096, 25600, 25600, "PLUS_TIMES"); 
}

TEST(SpGEMMvsvsTest, PlusTimesFloat4Test ) {

  test_spGEMM_vsvs_factory<float, float, float, float>(5, 32, 256, 128, "PLUS_TIMES"); 
}

TEST(SpGEMMvsdnTest, PlusTimesInt4Test) {

  test_spGEMM_vsdn_factory<int, int, int, int>(5, 32, 256, 32*32, "PLUS_TIMES"); 
}
TEST(SpGEMMvsdnTest, PlusTimesInt4TestMed) {

  test_spGEMM_vsdn_factory<int, int, int, int>(5, 256, 4096, 256*256, "PLUS_TIMES"); 
}

TEST( Reductions, PlusFloat) {
  test_reducefactoryUM<float>(4096, "PLUS");
}

TEST( Reductions, PlusDouble) {
  test_reducefactoryUM<double>(4096, "PLUS");
}

TEST( Reductions, MinFloat) {
  test_reducefactoryUM<float>(32,"MIN");
}

TEST( Reductions, MinInt) {
  test_reducefactoryUM<int>(32,"MIN");
}

TEST( Reductions, MaxInt) {
  test_reducefactoryUM<int>(32,"MAX");
}

