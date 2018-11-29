/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/GraphBLASExt/GxB_Delete.h"
#include "../../src/util/rmalloc.h"
#ifdef __cplusplus
}
#endif

class GxB_DeleteTest: public ::testing::Test {
  protected:
    static void SetUpTestCase() {
        // Use the malloc family for allocations
        Alloc_Reset();
        ASSERT_EQ(GrB_init(GrB_NONBLOCKING), GrB_SUCCESS);
        GxB_Global_Option_set(GxB_FORMAT, GxB_BY_COL); // all matrices in CSC format
        GxB_Global_Option_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse
    }

    static void TearDownTestCase()
    {
        GrB_finalize();
    }
};

TEST_F(GxB_DeleteTest, GxB_Delete) {
    // Create a 10X10 diagonal matrix.
    GrB_Matrix M;
    GrB_Matrix_new(&M, GrB_BOOL, 10, 10);
    
    // 1's along the diagonal.
    for(GrB_Index i = 0; i < 10; i++) {
        GrB_Matrix_setElement_BOOL(M, true, i, i);
    }

    GrB_Index nvals;
    GrB_Matrix_nvals(&nvals, M);
    ASSERT_EQ(nvals, 10);

    // Clear diagonal.
    for(GrB_Index i = 0; i < 10; i++) {
        GxB_Matrix_Delete(M, i, i);
        GrB_Matrix_nvals(&nvals, M);
        ASSERT_EQ(nvals, 10-1-i);
    }

    // Expecting an empty matrix.
    GrB_Matrix_nvals(&nvals, M);
    ASSERT_EQ(nvals, 0);
}