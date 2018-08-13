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

#include "../../src/index/index.h"
extern Index* buildIndex(Graph *g, const GrB_Matrix label_matrix, const char *label, const char *prop_str);

#ifdef __cplusplus
}
#endif

class IndexTest: public ::testing::Test {
  protected:
    size_t expected_n = 100;
    const char *label = "test_label";
    char *str_key = "string_prop";
    char *num_key = "num_prop";
    int label_id;
    Graph *g = build_test_graph();

    static void SetUpTestCase() {
      srand(time(NULL));
    }

    void TearDown() {
      Graph_Free(g);
    }

    Index* _indexCreate(Graph *g, const char *label, const char *property) {
      const GrB_Matrix label_matrix = Graph_GetLabelMatrix(g, label_id);
      return buildIndex(g, label_matrix, label, property);
    }

    Graph* build_test_graph() {
      // Allocate graph
      size_t n = expected_n;
      Graph *g = Graph_New(n);

      // Build a label matrix and add to graph
      // (this does not need to be associated with an actual label string)
      label_id = Graph_AddLabelMatrix(g);

      // Associate all nodes in graph with this label
      int label_ids[n];
      for (int i = 0; i < n; i ++) {
        label_ids[i] = label_id;
      }

      // Populate graph with nodes
      NodeIterator *it;
      Graph_CreateNodes(g, n, label_ids, &it);

      // Variables to store data for node properties
      Node *node;
      char *prop_keys[2];
      prop_keys[0] = str_key;
      prop_keys[1] = num_key;
      SIValue prop_vals[2];

      // Limiter for random values
      int denom = RAND_MAX / 20 + 1;

      for (int i = 0; i < n; i ++) {
        node = NodeIterator_Next(it);
        node->id = i;
        // Properties will have a random value between 1 and 20
        int prop_val = rand() / denom + 1;
        char str_prop[10];
        // Build a string and numeric property for each node out of the random value
        sprintf(str_prop, "%d", prop_val);
        prop_vals[0] = SI_StringVal(str_prop);
        prop_vals[1] = SI_DoubleVal(prop_val);

        Node_Add_Properties(node, 2, prop_keys, prop_vals);

        SIValue_Free(&prop_vals[0]);
      }
      NodeIterator_Free(it);

      return g;
    }
};

TEST_F(IndexTest, StringIndex) {
  // Index the label's string property
  Index* str_idx = _indexCreate(g, label, str_key);
  // Check the label and property tags on the index
  EXPECT_STREQ(label, str_idx->label);
  EXPECT_STREQ(str_key, str_idx->property);

  // The string list should have some values, and the numeric list should be empty
  EXPECT_GT(str_idx->string_sl->length, 0);
  EXPECT_EQ(str_idx->numeric_sl->length, 0);

  // Build an iterator from a constant filter - this will include all elements
  SIValue lb = SI_StringVal("");
  FT_FilterNode *filter = CreateConstFilterNode(label, str_key, GE, lb);
  IndexIter *iter = IndexIter_Create(str_idx, T_STRING);
  IndexIter_ApplyBound(iter, &filter->pred);
  FilterTree_Free(filter);

  NodeID *node_id;
  Node *cur;
  SIValue last_prop = lb;
  SIValue *cur_prop;

  int num_vals = 0;
  while ((node_id = IndexIter_Next(iter)) != NULL) {
    // Retrieve the node from the graph
    cur = Graph_GetNode(g, *node_id);
    // Retrieve the indexed property from the node
    cur_prop = GraphEntity_Get_Property((GraphEntity*)cur, str_key);
    // Values should be sorted in increasing value - duplicates are allowed
    EXPECT_LE(SIValue_Compare(last_prop, *cur_prop), 0);
    num_vals ++;
  }
  EXPECT_EQ(num_vals, expected_n);
  SIValue_Free(&lb);
  IndexIter_Free(iter);
  Index_Free(str_idx);
}

TEST_F(IndexTest, NumericIndex) {
  // Index the label's numeric property
  Index *num_idx = _indexCreate(g, label, num_key);
  // Check the label and property tags on the index
  EXPECT_STREQ(label, num_idx->label);
  EXPECT_STREQ(num_key, num_idx->property);

  // The numeric list should have some values, and the string list should be empty
  EXPECT_EQ(num_idx->string_sl->length,  0);
  EXPECT_GT(num_idx->numeric_sl->length, 0);

  // Build an iterator from a constant filter - this will include all elements
  SIValue lb = SI_DoubleVal(0);
  FT_FilterNode *filter = CreateConstFilterNode(label, num_key, GE, lb);
  IndexIter *iter = IndexIter_Create(num_idx, T_DOUBLE);
  IndexIter_ApplyBound(iter, &filter->pred);
  FilterTree_Free(filter);

  NodeID *node_id;
  Node *cur;
  SIValue last_prop = lb;
  SIValue *cur_prop;

  int num_vals = 0;
  while ((node_id = IndexIter_Next(iter)) != NULL) {
    // Retrieve the node from the graph
    cur = Graph_GetNode(g, *node_id);
    // Retrieve the indexed property from the node
    cur_prop = GraphEntity_Get_Property((GraphEntity*)cur, num_key);
    // Values should be sorted in increasing value - duplicates are allowed
    EXPECT_LE(SIValue_Compare(last_prop, *cur_prop), 0);
    num_vals ++;
  }
  EXPECT_EQ(num_vals, expected_n);

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
  Index *num_idx = _indexCreate(g, label, num_key);
  IndexIter *iter = IndexIter_Create(num_idx, T_DOUBLE);
  // Verify total number of values in index without range
  int prev_vals = count_iter_vals(iter);
  EXPECT_EQ(prev_vals, expected_n);
  IndexIter_Reset(iter);

  /* Apply a non-exclusive lower bound X */
  NodeID *node_id;
  Node *cur;
  SIValue *lb;
  int ctr = 0;
  // Find the value of the 10th element in the index
  while ((node_id = IndexIter_Next(iter)) != NULL) {
    ctr ++;
    if (ctr == 10) {
      cur = Graph_GetNode(g, *node_id);
      lb = GraphEntity_Get_Property((GraphEntity*)cur, num_key);
    }
  }

  FT_FilterNode *lb_filter_ge = CreateConstFilterNode(label, num_key, GE, *lb);
  IndexIter_Reset(iter);
  IndexIter_ApplyBound(iter, &lb_filter_ge->pred);
  /* Lower bound should reduce the number of values iterated over */
  int cur_vals = count_iter_vals(iter);
  EXPECT_LT(cur_vals, prev_vals);

  /* Set the same lower bound, but exclusive.
   * Number of values should again be reduced. */
  IndexIter_Reset(iter);
  FT_FilterNode *lb_filter_gt = CreateConstFilterNode(label, num_key, GT, *lb);
  IndexIter_ApplyBound(iter, &lb_filter_gt->pred);
  cur_vals = count_iter_vals(iter);
  EXPECT_LT(cur_vals, prev_vals);

  /* Apply a non-exclusive upper bound at next indexed value */
  prev_vals = cur_vals;
  IndexIter_Reset(iter);
  SIValue *ub;
  while ((node_id = IndexIter_Next(iter)) != NULL) {
    cur = Graph_GetNode(g, *node_id);
    ub = GraphEntity_Get_Property((GraphEntity*)cur, num_key);
    if (ub->doubleval > lb->doubleval) break;
  }
  FT_FilterNode *ub_filter_le = CreateConstFilterNode(label, num_key, LE, *ub);
  IndexIter_ApplyBound(iter, &ub_filter_le->pred);
  /* Upper bound should reduce the number of values iterated over */
  cur_vals = count_iter_vals(iter);
  EXPECT_LT(cur_vals, prev_vals);
  prev_vals = cur_vals;

  /* Set the same upper bound, but exclusive.
   * Number of values should again be reduced. */
  IndexIter_Reset(iter);
  FT_FilterNode *ub_filter_lt = CreateConstFilterNode(label, num_key, LT, *ub);
  IndexIter_ApplyBound(iter, &ub_filter_lt->pred);
  cur_vals = count_iter_vals(iter);
  EXPECT_LT(cur_vals, prev_vals);

  /* Set an upper bound beneath the current lower bound.
   * Number of values should be 0. */
  IndexIter_Reset(iter);
  SIValue ub_last = SI_DoubleVal(lb->doubleval - 1);
  FT_FilterNode *ub_filter_last = CreateConstFilterNode(label, num_key, LT, ub_last);
  IndexIter_ApplyBound(iter, &ub_filter_last->pred);
  cur_vals = count_iter_vals(iter);
  EXPECT_EQ(cur_vals, 0);

  free(lb_filter_gt);
  free(lb_filter_ge);
  free(ub_filter_le);
  free(ub_filter_lt);
  free(ub_filter_last);
  IndexIter_Free(iter);
}

