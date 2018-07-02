#include <stdio.h>
#include <time.h>
#include "assert.h"
#include "../../src/index/index.h"

// These functions are defined in index.c, but not declared in index.h
extern IndexCreateIter* IndexCreateIter_CreateFromFilter(Index *idx, FT_PredicateNode *filter);
extern void _index_free(Index *idx);

const char *label = "test_label";
char *str_key = "string_prop";
char *num_key = "num_prop";
int label_id;

Index* test_index_create(Graph *g, const char *label, const char *property) {
  GrB_Matrix label_matrix = g->labels[label_id];
  return buildIndex(g, label_matrix, label, property);
}

Graph* build_test_graph() {
  // Allocate graph
  size_t n = 100;
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

  for(int i = 0; i < n; i++) {
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

void validate_string_index(Graph *g, Index *idx) {
  // Check the label and property tags on the index
  assert(strcmp(label, idx->label) == 0);
  assert(strcmp(str_key, idx->property) == 0);

  // The string list should have some values, and the numeric list should be empty
  assert(idx->string_sl->length > 0);
  assert(idx->numeric_sl->length == 0);

  // Build an iterator from a constant filter - this will include all elements
  SIValue lb = SI_StringVal("");
  FT_FilterNode *filter = CreateConstFilterNode(label, str_key, GE, lb);
  IndexCreateIter *iter = IndexCreateIter_CreateFromFilter(idx, &filter->pred);
  FilterTree_Free(filter);

  GrB_Index node_id;
  Node *cur;
  SIValue last_prop = lb;
  SIValue *cur_prop;

  // int num_vals = 0;
  while ((node_id = (GrB_Index)IndexCreateIter_Next(iter))) {
    // Retrieve the node from the graph
    cur = Graph_GetNode(g, node_id);
    // Retrieve the indexed property from the node
    cur_prop = GraphEntity_Get_Property((GraphEntity*)cur, str_key);
    // Values should be sorted in increasing value - duplicates are allowed 
    assert(SIValue_Compare(last_prop, *cur_prop) <= 0);
    // num_vals ++;
  }
  // assert(num_vals == 100);
  SIValue_Free(&lb);
  IndexCreateIter_Free(iter);
}

void validate_numeric_index(Graph *g, Index *idx) {
  // Check the label and property tags on the index
  assert(strcmp(label, idx->label) == 0);
  assert(strcmp(num_key, idx->property) == 0);

  // The numeric list should have some values, and the string list should be empty
  assert(idx->string_sl->length == 0);
  assert(idx->numeric_sl->length > 0);

  // Build an iterator from a constant filter - this will include all elements
  SIValue lb = SI_DoubleVal(0);
  FT_FilterNode *filter = CreateConstFilterNode(label, str_key, GE, lb);
  IndexCreateIter *iter = IndexCreateIter_CreateFromFilter(idx, &filter->pred);
  FilterTree_Free(filter);


  GrB_Index node_id;
  Node *cur;
  SIValue last_prop = lb;
  SIValue *cur_prop;

  // int num_vals = 0;
  while ((node_id = (GrB_Index)IndexCreateIter_Next(iter))) {
    // Retrieve the node from the graph
    cur = Graph_GetNode(g, node_id);
    // Retrieve the indexed property from the node
    cur_prop = GraphEntity_Get_Property((GraphEntity*)cur, str_key);
    // Values should be sorted in increasing value - duplicates are allowed 
    assert(SIValue_Compare(last_prop, *cur_prop) <= 0);
    // num_vals ++;
    // SIValue_Print(stdout, (SIValue*)cur_prop);
  }
  // assert(num_vals == 100);
  // assert(num_vals == 100);
  IndexCreateIter_Free(iter);
}

int main() {
  srand(time(NULL));

  Graph *g = build_test_graph();

  // Build separate indices on the string and numeric property of each node
  Index *str_index = test_index_create(g, label, str_key);
  Index *num_index = test_index_create(g, label, num_key);

  validate_string_index(g, str_index);
  validate_numeric_index(g, num_index);

  _index_free(str_index);
  _index_free(num_index);

  Graph_Free(g);

  printf("test_index - PASS!\n");
}