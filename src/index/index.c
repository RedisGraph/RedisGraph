/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "index.h"
#include "../util/rmalloc.h"

// Given a value type, return the matching skiplist from an index.
static inline skiplist* _select_skiplist(const Index *idx, const SIType t) {
  if (t & SI_STRING) {
    return idx->string_sl;
  } else if (t & SI_NUMERIC) {
    return idx->numeric_sl;
  }
  return NULL;
}

//------------------------------------------------------------------------------
// Function pointers for skiplist routines
//------------------------------------------------------------------------------
int compareNodes(NodeID a, NodeID b) {
  return a - b;
}

int compareStrings(SIValue *a, SIValue *b) {
  return strcmp(a->stringval, b->stringval);
}

int compareNumerics(SIValue *a, SIValue *b) {
  double diff = a->doubleval - b->doubleval;
  return COMPARE_RETVAL(diff);
}

/* The index must maintain its own copy of the indexed SIValue
 * so that it becomes outdated but not broken by updates to the property. */
SIValue* cloneKey(SIValue *property) {
  SIValue *clone = rm_malloc(sizeof(SIValue));
  *clone = SI_Clone(*property);
  return clone;
}

void freeKey(SIValue *key) {
  SIValue_Free(key);
  rm_free(key);
}

//------------------------------------------------------------------------------
// Index creation functions
//------------------------------------------------------------------------------
void initializeSkiplists(Index *index) {
  index->string_sl = skiplistCreate(compareStrings, compareNodes, cloneKey, freeKey);
  index->numeric_sl = skiplistCreate(compareNumerics, compareNodes, cloneKey, freeKey);
}

/* Index_Create allocates an Index object and populates it with all unique IDs and values
 * that possess the provided label and property. */
Index* Index_Create(Graph *g, int label_id, const char *label, const char *prop_str) {
  const GrB_Matrix label_matrix = Graph_GetLabel(g, label_id);
  GxB_MatrixTupleIter *it;
  GxB_MatrixTupleIter_new(&it, label_matrix);

  Index *index = rm_malloc(sizeof(Index));

  index->label = rm_strdup(label);
  index->property = rm_strdup(prop_str);

  initializeSkiplists(index);

  Node node;
  EntityProperty *prop;

  skiplist *sl;
  NodeID node_id;
  GraphEntity *entity;

  int found;
  int prop_index = 0;

  while(true) {
    bool depleted = false;
    GxB_MatrixTupleIter_next(it, NULL, &node_id, &depleted);
    if(depleted) break;
    Graph_GetNode(g, node_id, &node);
    // If the sought property is at a different offset than it occupied in the previous node,
    // then seek and update
    if (strcmp(prop_str, ENTITY_PROPS(&node)[prop_index].name)) {
      found = 0;
      for (int i = 0; i < ENTITY_PROP_COUNT(&node); i ++) {
        prop = ENTITY_PROPS(&node) + i;
        if (!strcmp(prop_str, prop->name)) {
          prop_index = i;
          found = 1;
          break;
        }
      }
    } else {
      found = 1;
    }
    // The targeted property does not exist on this node
    if (!found) continue;

    prop = ENTITY_PROPS(&node) + prop_index;
    // This value will be cloned within the skiplistInsert routine if necessary
    SIValue *key = &prop->value;

    sl = _select_skiplist(index, key->type);
    if (!sl) continue; // Value was of a type not supported by indices.
    skiplistInsert(sl, key, node_id);
  }

  GxB_MatrixTupleIter_free(it);

  return index;
}

//------------------------------------------------------------------------------
// Index updates
//------------------------------------------------------------------------------

void Index_DeleteNode(Index *idx, NodeID node, SIValue *val) {
  skiplist *sl = _select_skiplist(idx, val->type);
  if (!sl) return; // Value was of a type not supported by indices.
  skiplistDelete(sl, val, &node);
}
 void Index_InsertNode(Index *idx, NodeID node, SIValue *val) {
  skiplist *sl = _select_skiplist(idx, val->type);
  if (!sl) return; // Value was of a type not supported by indices.
  skiplistInsert(sl, val, node);
}

//------------------------------------------------------------------------------
// Index iterator functions
//------------------------------------------------------------------------------

/* Generate an iterator with no lower or upper bound. */
IndexIter* IndexIter_Create(Index *idx, SIType type) {
  skiplist *sl = type & SI_STRING ? idx->string_sl : idx->numeric_sl;
  return skiplistIterateAll(sl);
}

/* Apply a filter to an iterator, modifying the appropriate bound if
 * it narrows the iterator range.
 * Returns 1 if the filter was a comparison type that can be translated into a bound
 * (effectively, any type but '!='), which indicates that it is now redundant. */
bool IndexIter_ApplyBound(IndexIter *iter, SIValue *bound, int op) {
  return skiplistIter_UpdateBound(iter, bound, op);
}

NodeID* IndexIter_Next(IndexIter *iter) {
  return skiplistIterator_Next(iter);
}

void IndexIter_Reset(IndexIter *iter) {
  skiplistIterate_Reset(iter);
}

void IndexIter_Free(IndexIter *iter) {
  skiplistIterate_Free(iter);
}

void Index_Free(Index *idx) {
  skiplistFree(idx->string_sl);
  skiplistFree(idx->numeric_sl);
  rm_free(idx->label);
  rm_free(idx->property);
  rm_free(idx);
}

