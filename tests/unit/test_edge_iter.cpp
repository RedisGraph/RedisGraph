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

#include "../../src/graph/edge_iterator.h"

#ifdef __cplusplus
}
#endif

class EdgeIterTest: public ::testing::Test {
  protected:
    static void SetUpTestCase() {      
    }

    static void TearDownTestCase() {
    }   
};

TEST_F(EdgeIterTest, NewEdgeIter) {
    EdgeIterator *it = EdgeIterator_New();

    EXPECT_EQ(it->edgeIdx, 0);
    EXPECT_EQ(it->edgeCount, 0);
    EXPECT_EQ(it->edgeCap, EDGE_ITERATOR_EDGE_CAP);
    EXPECT_TRUE(it->edges != NULL);

    EdgeIterator_Free(it);
}

TEST_F(EdgeIterTest, ResetEdgeIter) {
    EdgeIterator *it = EdgeIterator_New();
    it->edgeCount = 16;
    it->edgeIdx = 5;

    EdgeIterator_Reset(it);
    EXPECT_EQ(it->edgeIdx, 0);
    EXPECT_EQ(it->edgeCount, 16);
    EXPECT_EQ(it->edgeCap, EDGE_ITERATOR_EDGE_CAP);
}

TEST_F(EdgeIterTest, ReuseMatrixTest) {
    EdgeIterator *it = EdgeIterator_New();
    it->edgeCount = 16;
    it->edgeIdx = 5;

    EdgeIterator_Reuse(it);
    EXPECT_EQ(it->edgeIdx, 0);
    EXPECT_EQ(it->edgeCount, 0);
    EXPECT_EQ(it->edgeCap, EDGE_ITERATOR_EDGE_CAP);
}
