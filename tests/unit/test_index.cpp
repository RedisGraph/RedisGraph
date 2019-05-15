/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/graph/graph.h"
#include "../../src/parser/newast.h"
#include "../../src/index/index.h"
#include "../../src/util/rmalloc.h"

#ifdef __cplusplus
}
#endif

class IndexTest: public ::testing::Test {
  protected:
    size_t expected_n = 100;
    int label_id;
    char *label = "test_label";

    Attribute_ID str_key_id = 0;
    char *str_key = "string_prop";

    Attribute_ID num_key_id = 1;
    char *num_key = "num_prop";

    Graph *g = build_test_graph();

    static void SetUpTestCase() {
      // Use seeded generator, as it is possible to create a graph with
      // too few unique values to pass all tests.
      srand(0);

      // Use the malloc family for allocations
      Alloc_Reset();
      ASSERT_EQ(GrB_init(GrB_NONBLOCKING), GrB_SUCCESS);
      GxB_Global_Option_set(GxB_FORMAT, GxB_BY_COL); // all matrices in CSC format
      GxB_Global_Option_set(GxB_HYPER, GxB_NEVER_HYPER); // matrices are never hypersparse
    }

    static void TearDownTestCase() {
        GrB_finalize();
    }
    
    void TearDown() {
      Graph_Free(g);
    }

    Graph* build_test_graph() {
      // Allocate graph
      size_t n = expected_n;
      Graph *g = Graph_New(n, n);
      Graph_AcquireWriteLock(g);

      // Build a label matrix and add to graph
      // (this does not need to be associated with an actual label string)
      label_id = Graph_AddLabel(g);
      Graph_AllocateNodes(g, n);

      // Populate graph with nodes
      Node node;
      for(int i = 0; i < n; i++) {
        Graph_CreateNode(g, label_id, &node);
        // Limiter for random values
        int denom = RAND_MAX / 20 + 1;

        // Properties will have a random value between 1 and 20
        int prop_val = rand() / denom + 1;
        char str_prop[10];

        // Build a string and numeric property for each node out of the random value
        sprintf(str_prop, "%d", prop_val);
        SIValue val = SI_DuplicateStringVal(str_prop);
        GraphEntity_AddProperty((GraphEntity*)&node, str_key_id, val);

        val = SI_DoubleVal(prop_val);
        GraphEntity_AddProperty((GraphEntity*)&node, num_key_id, val);
      }

      Graph_ReleaseLock(g);
      return g;
    }
};

TEST_F(IndexTest, StringIndex) {
  // Index the label's string property
  Index* str_idx = Index_Create(g, label, label_id, str_key, str_key_id);
  // Check the label and property tags on the index
  ASSERT_STREQ(label, str_idx->label);
  ASSERT_STREQ(str_key, str_idx->attribute);
  ASSERT_EQ(str_key_id, str_idx->attr_id);

  // The string list should have some values, and the numeric list should be empty
  ASSERT_GT(str_idx->string_sl->length, 0);
  ASSERT_EQ(str_idx->numeric_sl->length, 0);

  // Build an iterator from a constant filter - this will include all elements
  SIValue lb = SI_ConstStringVal("");

  IndexIter *iter = IndexIter_Create(str_idx, T_STRING);
  IndexIter_ApplyBound(iter, &lb, OP_GE);

  NodeID *node_id;
  Node cur;
  SIValue last_prop = lb;
  SIValue *cur_prop;

  int num_vals = 0;
  while ((node_id = IndexIter_Next(iter)) != NULL) {
    // Retrieve the node from the graph
    Graph_GetNode(g, *node_id, &cur);
    // Retrieve the indexed property from the node
    cur_prop = GraphEntity_GetProperty((GraphEntity*)&cur, str_key_id);
    // Values should be sorted in increasing value - duplicates are allowed
    ASSERT_LE(SIValue_Compare(last_prop, *cur_prop), 0);
    num_vals ++;
  }
  ASSERT_EQ(num_vals, expected_n);

  SIValue_Free(&lb);
  IndexIter_Free(iter);
  Index_Free(str_idx);
}

TEST_F(IndexTest, NumericIndex) {
  // Index the label's numeric property
  Index *num_idx = Index_Create(g, label, label_id, num_key, num_key_id);
  // Check the label and property tags on the index
  ASSERT_STREQ(label, num_idx->label);
  ASSERT_STREQ(num_key, num_idx->attribute);
  ASSERT_EQ(num_key_id, num_idx->attr_id);
  
  // The numeric list should have some values, and the string list should be empty
  ASSERT_EQ(num_idx->string_sl->length,  0);
  ASSERT_GT(num_idx->numeric_sl->length, 0);

  // Build an iterator from a constant filter - this will include all elements
  SIValue lb = SI_DoubleVal(0);
  IndexIter *iter = IndexIter_Create(num_idx, T_DOUBLE);
  IndexIter_ApplyBound(iter, &lb, OP_GE);

  NodeID *node_id;
  Node cur;
  SIValue last_prop = lb;
  SIValue *cur_prop;

  int num_vals = 0;
  while ((node_id = IndexIter_Next(iter)) != NULL) {
    // Retrieve the node from the graph
    Graph_GetNode(g, *node_id, &cur);
    // Retrieve the indexed property from the node
    cur_prop = GraphEntity_GetProperty((GraphEntity*)&cur, num_key_id);
    // Values should be sorted in increasing value - duplicates are allowed
    ASSERT_LE(SIValue_Compare(last_prop, *cur_prop), 0);
    num_vals ++;
  }
  ASSERT_EQ(num_vals, expected_n);

  IndexIter_Free(iter);
  Index_Free(num_idx);
}

static int count_iter_vals(IndexIter *iter) {
  int ctr = 0;
  NodeID *id;
  while ((id = IndexIter_Next(iter)) != NULL) {
    ctr ++;
  }
  return ctr;
}

/* Validate the progressive application of iterator bounds
 * on the numeric skiplist. */
TEST_F(IndexTest, IteratorBounds) {
  Index *num_idx = Index_Create(g, label, label_id, num_key, num_key_id);
  IndexIter *iter = IndexIter_Create(num_idx, T_DOUBLE);
  // Verify total number of values in index without range
  int prev_vals = count_iter_vals(iter);
  ASSERT_EQ(prev_vals, expected_n);
  IndexIter_Reset(iter);

  /* Apply a non-exclusive lower bound X */
  NodeID *node_id;
  Node cur;
  SIValue *lb;
  int ctr = 0;
  // TODO ensure lower bound is not still first value
  // Find the value of the 10th element in the index
  while ((node_id = IndexIter_Next(iter)) != NULL) {
    ctr ++;
    if (ctr == 10) {
      Graph_GetNode(g, *node_id, &cur);
      lb = GraphEntity_GetProperty((GraphEntity*)&cur, num_key_id);
    }
  }

  IndexIter_Reset(iter);
  IndexIter_ApplyBound(iter, lb, OP_GE);
  /* Lower bound should reduce the number of values iterated over */
  int cur_vals = count_iter_vals(iter);
  ASSERT_LT(cur_vals, prev_vals);

  /* Set the same lower bound, but exclusive.
   * Number of values should again be reduced. */
  IndexIter_Reset(iter);
  IndexIter_ApplyBound(iter, lb, OP_GT);
  cur_vals = count_iter_vals(iter);
  ASSERT_LT(cur_vals, prev_vals);

  /* Apply a non-exclusive upper bound at next indexed value */
  prev_vals = cur_vals;
  IndexIter_Reset(iter);
  SIValue *ub;
  while ((node_id = IndexIter_Next(iter)) != NULL) {
    Graph_GetNode(g, *node_id, &cur);
    ub = GraphEntity_GetProperty((GraphEntity*)&cur, num_key_id);
    if (ub->doubleval > lb->doubleval) break;
  }
  IndexIter_ApplyBound(iter, ub, OP_LE);
  /* Upper bound should reduce the number of values iterated over */
  cur_vals = count_iter_vals(iter);
  ASSERT_LT(cur_vals, prev_vals);
  prev_vals = cur_vals;

  /* Set the same upper bound, but exclusive.
   * Number of values should again be reduced. */
  IndexIter_Reset(iter);
  IndexIter_ApplyBound(iter, ub, OP_LT);
  cur_vals = count_iter_vals(iter);
  ASSERT_LT(cur_vals, prev_vals);

  /* Set an upper bound beneath the current lower bound.
   * Number of values should be 0. */
  IndexIter_Reset(iter);
  SIValue ub_last = SI_DoubleVal(lb->doubleval - 1);
  IndexIter_ApplyBound(iter, &ub_last, OP_LT);
  cur_vals = count_iter_vals(iter);
  ASSERT_EQ(cur_vals, 0);

  IndexIter_Free(iter);
  Index_Free(num_idx);
}
