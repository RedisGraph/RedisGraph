#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/util/skiplist.h"
#include "../../src/value.h"
#include "../../src/graph/node.h"

// Numeric key comparator
int cmpDoubleKeys(void *p1, void *p2, void *ctx) {
  int a = *(double *)p1;
  int b = *(double *)p2;

  if (a > b) {
    return 1;
  } else if (a < b) {
    return -1;
  } else {
    return 0;
  }
}

// Numeric value comparator
int cmpDoubleVals(const void *p1, const void *p2) {
  int a = *(double *)p1;
  int b = *(double *)p2;

  if (a > b) {
    return 1;
  } else if (a < b) {
    return -1;
  } else {
    return 0;
  }
}

// String key comparator
int cmpStrKeys(void *a, void *b, void *ctx) {
  return strcmp(a, b);
}

// String value comparator
int cmpStrVals(const void *a, const void *b) {
  return strcmp(a, b);
}

// Verify the allocation and deletion properties of the vals array in a skiplistNode
void test_vals_allocation() {
  skiplist *sl = skiplistCreate(cmpStrKeys, NULL, cmpDoubleVals, NULL, NULL);
  double i;
  for (i = 0; i < 100; i ++) {
    skiplistInsert(sl, "a", &i);
  }
  skiplistNode *node = skiplistFind(sl, "a");

  // The values array initially increases by powers of 2
  assert(node->valsAllocated == 128);

  for (i = 0; i < 80; i ++) {
    skiplistDelete(sl, "a", &i);
  }

  // When the vals array shrinks to <= 25% full, we reduce its capacity by half.
  // In this case, we populate the array to contain 100 values and have space for 128.
  // We then delete 4 of every 5 values. When numVals == 32, we shrink the array
  // to allow for 64 elements.
  assert(node->valsAllocated == 64);
  // When we finish our deletions, there are 20 values remaining (the array length would be
  // cut in half again if we deleted 4 more values).
  assert(node->numVals == 20);

  skiplistFree(sl);
}

// Verify lexicographic sorting of string keys
void test_string_sorts(void) {

  char *words[] = {"foo",  "bar",     "zap",    "pomo",
                   "pera", "arancio", "limone", NULL};

  char *vals[] = {"foo val", "bar val", "zap val", "pomo val",
                  "pera val", "arancio val", "limone val", NULL};

  skiplist *string_sl = skiplistCreate(cmpStrKeys, NULL, cmpStrVals, NULL, NULL);

  for (int i = 0; words[i] != NULL; i ++) {
    skiplistInsert(string_sl, words[i], vals[i]);
  }

  char *val, *prevVal = "";
  int str_ret;
  // LHS value should always return negative
  while ((val = skiplistPopHead(string_sl)) != NULL) {
    str_ret = strcmp(prevVal, val);
    assert(str_ret < 0);
    prevVal = val;
  }

  skiplistFree(string_sl);
}

// Verify skiplist operations on numeric keys
void test_numeric_sorts(void) {

  double keys[] = {5.5, 0, -30.2, 7, 1, 2, -1.5};

  // vals are defined as the order the keys should be in after sorting
  double vals[] = {5, 2, 0, 6, 3, 4, 1};

  skiplist *numeric_sl = skiplistCreate(cmpDoubleKeys, NULL, cmpDoubleVals, NULL, NULL);

  for (int i = 0; i < 7; i ++) {
    skiplistInsert(numeric_sl, keys + i, vals + i);
  }

  // Search for a key not in skiplist
  skiplistNode *ret;
  double non_existent_key = -2;
  double non_existent_val = -5;
  ret = skiplistFind(numeric_sl, &non_existent_key);
  assert (ret == NULL);

  // Search for a key in skiplist
  ret = skiplistFind(numeric_sl, &keys[5]);
  assert (*(double*)ret->vals[0] == vals[5]);

  int delete_result;
  // Attempt to delete from a key that does not exist
  delete_result = skiplistDelete(numeric_sl, &non_existent_key, &non_existent_val);
  assert(delete_result == 0);

  // Attempt to delete a non-existent value from a real key
  delete_result = skiplistDelete(numeric_sl, &keys[0], &non_existent_val);
  assert(delete_result == 0);

  // Attempt to delete a non-existent skiplist key
  delete_result = skiplistDelete(numeric_sl, &non_existent_key, NULL);
  assert(delete_result == 0);

  // Retrieve the first node greater than an arbitrary number
  double threshold = 0.5;
  ret = skiplistFindAtLeast(numeric_sl, &threshold, 1);
  assert (*(double*)ret->key == 1);

  // Verify that skiplist is sorted on numeric keys
  double lastval = -1;
  double *retval;
  skiplistIterator *iter = skiplistIterateAll(numeric_sl);
  while ((retval = skiplistIterator_Next(iter)) != NULL) {
    assert(lastval + 1 == *(double *)retval);
    lastval = *retval;
  }

  // Delete a node from the skiplist
  delete_result = skiplistDelete(numeric_sl, &keys[3], NULL);
  assert(delete_result == 1);
  ret = skiplistFind(numeric_sl, &keys[3]);
  assert (ret == NULL);

  // Verify that the delete operation removes nodes from the skiplist
  skiplistIterate_Reset(iter);
  while ((retval = skiplistIterator_Next(iter)) != NULL) {
    assert(*(double *)retval != keys[3]);
  }
  skiplistIterate_Free(iter);

  // Iterate over a range of keys (-INF, 0]
  double key_val = 0;
  iter = skiplistIterateRange(numeric_sl, NULL, &key_val, 0, 1);

  // For this skiplist, this iterator should solely contain the first 2 key-value pairs
  retval = skiplistIterator_Next(iter);
  assert(*retval == 0);
  retval = skiplistIterator_Next(iter);
  assert(*retval == 1);
  retval = skiplistIterator_Next(iter);
  assert(retval == NULL);
  skiplistIterate_Free(iter);

  // Iterate over a range of keys [0, 5]
  key_val = 0;
  double final_key_val = 5;
  iter = skiplistIterateRange(numeric_sl, &key_val, &final_key_val, 0, 1);

  retval = skiplistIterator_Next(iter);
  assert(*retval == 2);
  retval = skiplistIterator_Next(iter);
  assert(*retval == 3);
  retval = skiplistIterator_Next(iter);
  assert(*retval == 4);
  retval = skiplistIterator_Next(iter);
  assert(retval == NULL);
  skiplistIterate_Free(iter);

  skiplistFree(numeric_sl);
}

// Verify that the skiplist iterator passes over all elements (in order)
void test_sl_iterator(void) {
  skiplist *string_sl = skiplistCreate(cmpStrKeys, NULL, cmpDoubleVals, NULL, NULL);
  skiplistNode *x;
  double first = 1, second = 2, third = 3;
  skiplistInsert(string_sl, "c_third", &third);
  skiplistInsert(string_sl, "a_first", &first);
  skiplistInsert(string_sl, "b_second", &second);

  skiplistIterator *iter = skiplistIterateAll(string_sl);

  x = skiplistIterator_Next(iter);
  assert (*(double *) x == 1);

  x = skiplistIterator_Next(iter);
  assert (*(double *) x == 2);

  x = skiplistIterator_Next(iter);
  assert (*(double *) x == 3);

  x = skiplistIterator_Next(iter);
  assert (x == NULL);

  skiplistIterate_Free(iter);
  skiplistFree(string_sl);
}

int main(void) {
  test_vals_allocation();
  test_string_sorts();
  test_numeric_sorts();
  test_sl_iterator();

  printf("test_skiplist - PASS!\n");
}
