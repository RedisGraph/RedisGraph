#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/util/skiplist.h"
#include "../../src/value.h"
#include "../../src/graph/node.h"

char *words[] = {"foo",  "bar",     "zap",    "pomo",
                 "pera", "arancio", "limone", NULL};

const char *node_label = "default_label";
char *prop_key = "default_prop_key";

// NOTE - these 'test*' functions are copies of those seen in 'index.c',
// which can only be accessed from within the scope of index operations

// Skiplist comparator functions
int testCompareNodes(const void *a, const void *b) {
  return ((Node*)a)->id - ((Node*)b)->id;
}

int testCompareStrings(void *a, void *b, void *ctx) {
  return strcmp(((SIValue*)a)->stringval, ((SIValue*)b)->stringval);
}
int testCompareNumerics(void *p1, void *p2, void *ctx) {
  double a, b;
  SIValue_ToDouble(p1, &a);
  SIValue_ToDouble(p2, &b);
  return a - b;
}

void testCloneKey(void **property) {
  SIValue *redirect = *property;
  *redirect = SI_Clone(*redirect);
}

void testFreeKey(void *key) {
  SIValue_Free(key);
}

skiplist* build_skiplist(void) {
  skiplist *sl = skiplistCreate(testCompareStrings, NULL, testCompareNodes, testCloneKey, testFreeKey);
  Node *node_a, *node_b;

  for (long i = 0; words[i] != NULL; i ++) {
    node_a = Node_New(10 + i, node_label);
    SIValue *node_a_prop = malloc(sizeof(SIValue));
    *node_a_prop = SIValue_FromString(words[i]);
    Node_Add_Properties(node_a, 1, &prop_key, node_a_prop);
    skiplistInsert(sl, node_a_prop, node_a);

    node_b = Node_New(i, node_label);
    SIValue *node_b_prop = malloc(sizeof(SIValue));
    *node_b_prop = SIValue_FromString(words[6 - i]);
    Node_Add_Properties(node_b, 1, &prop_key, node_b_prop);
    skiplistInsert(sl, node_b_prop, node_b);
  }

  return sl;
}

// Update key-value pair
skiplistNode* update_skiplist(skiplist *sl, void *val, void *old_key, void *new_key) {
  // Ignore the return value from skiplistDelete
  skiplistDelete(sl, old_key, val);
  return skiplistInsert(sl, new_key, val);
}

void test_skiplist_range(void) {
  skiplist *sl = skiplistCreate(testCompareNumerics, NULL, testCompareNodes, testCloneKey, testFreeKey);
  Node *cur_node;
  SIValue cur_prop;

  char *keys[] = {"5.5", "0", "-30.2", "7", "1", "2", "-1.5", NULL};

  // The IDs we will assign to the Node values in the skiplist
  // (defined as the order the keys should be in after sorting)
  long ids[] = {5, 2, 0, 6, 3, 4, 1};

  for (long i = 0; keys[i] != NULL; i ++) {
    cur_node = Node_New(ids[i], node_label);
    cur_prop = SIValue_FromString(keys[i]);
    Node_Add_Properties(cur_node, 1, &prop_key, &cur_prop);
    skiplistInsert(sl, &cur_prop, cur_node);
  }

  long last_id = 3;
  // Iterate over a range of keys [1, INF)
  SIValue min = SI_DoubleVal(1);
  skiplistIterator *iter = skiplistIterateRange(sl, &min, NULL, 1, 0);
  while ((cur_node = skiplistIterator_Next(iter)) != NULL) {
    assert(last_id + 1 == cur_node->id);
    last_id = cur_node->id;
  }

  skiplistFree(sl);
  skiplistIterate_Free(iter);
}

void test_skiplist_delete(void) {
  int delete_result;

  skiplist *sl = build_skiplist();

  SIValue prop_to_delete = SIValue_FromString(words[2]);
  skiplistNode *old_skiplist_node = skiplistFind(sl, &prop_to_delete);
  Node *node_to_delete = old_skiplist_node->vals[0];
  prop_to_delete = SIValue_FromString(words[3]);

  // Attempt to delete a non-existent key-value pair
  delete_result = skiplistDelete(sl, &prop_to_delete, node_to_delete);
  assert(delete_result == 0);

  // Attempt to delete a non-existent skiplist key
  prop_to_delete = SIValue_FromString("not_a_key");
  delete_result = skiplistDelete(sl, &prop_to_delete, NULL);
  assert(delete_result == 0);

  // Delete a single value from the skiplist
  delete_result = skiplistDelete(sl, Node_Get_Property(node_to_delete, prop_key), node_to_delete);
  assert(delete_result == 1);

  // Delete full nodes from the skiplist - a single key and all values that share it
  prop_to_delete = SIValue_FromString(words[3]);
  delete_result = skiplistDelete(sl, &prop_to_delete, NULL);
  assert(delete_result == 1);

  // Verify that the skiplistNode has been deleted
  void *search_result = skiplistFind(sl, &prop_to_delete);
  assert(search_result == NULL);

  skiplistFree(sl);
}

void test_skiplist_update(void) {
  skiplistNode *search_result;
  skiplist *sl = build_skiplist();
  SIValue find_prop = SIValue_FromString(words[2]);
  skiplistNode *old_skiplist_node = skiplistFind(sl, &find_prop);
  Node *node_to_update = old_skiplist_node->vals[0];

  SIValue old_prop = *Node_Get_Property(node_to_update, prop_key);

  // Update the skiplist to store a different property
  SIValue new_prop = SIValue_FromString("updated_val");
  skiplistNode *new_skiplist_node = update_skiplist(sl, node_to_update, &old_prop, &new_prop);

  // The new skiplistNode should have the new key
  assert(!strcmp(((SIValue *)new_skiplist_node->key)->stringval, "updated_val"));

  // The old key-value pair must have been deleted
  int delete_result = skiplistDelete(sl, & old_prop, node_to_update);
  assert(delete_result == 0);

  // The new key-value pair can be found
  int found_index = -1;
  search_result = skiplistFind(sl, &new_prop);
  if (search_result) {
    for (int i = 0; i < search_result->numVals; i ++) {
      if (testCompareNodes(search_result->vals[i], node_to_update) == 0) {
        found_index = i;
        break;
      }
    }
    assert(found_index >= 0);
  }

  skiplistFree(sl);
}

int main(void) {
  test_skiplist_delete();
  test_skiplist_update();
  test_skiplist_range();

  printf("test_skiplist_graph - PASS!\n");
}
