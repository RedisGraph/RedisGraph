/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "index.h"
#include "index_type.h"

RedisModuleKey* _index_LookupKey(RedisModuleCtx *ctx, const char *graph, size_t index_id, bool write_access) {
  char *strKey;
  int keylen = asprintf(&strKey, "%s_%s_%lu", INDEX_PREFIX, graph, index_id);

  RedisModuleString *rmIndexId = RedisModule_CreateString(ctx, strKey, keylen);
  free(strKey);

  int mode = write_access ? REDISMODULE_WRITE : REDISMODULE_READ;
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rmIndexId, mode);
  RedisModule_FreeString(ctx, rmIndexId);

  return key;
}

/* Memory management and comparator functions that get attached to
 * string and numeric skiplists as function pointers. */
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
  SIValue *clone = malloc(sizeof(SIValue));
  *clone = SI_Clone(*property);
  return clone;
}

void freeKey(SIValue *key) {
  SIValue_Free(key);
  free(key);
}

/* Construct key and retrieve index from Redis keyspace */
Index* Index_Get(RedisModuleCtx *ctx, const char *graph, const char *label, char *property) {
  Index *idx = NULL;

  LabelStore *store = LabelStore_Get(ctx, STORE_NODE, graph, label);
  if (!store) return NULL;

  size_t *index_id = TrieMap_Find(store->properties, property, strlen(property));

  // index_id will be NULL if the label-property pair is not indexed
  if (!index_id || index_id == TRIEMAP_NOTFOUND) return NULL;

  // Open key with read-only access
  RedisModuleKey *key = _index_LookupKey(ctx, graph, *index_id, false);

  if (RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType) {
    idx = RedisModule_ModuleTypeGetValue(key);
  }

  RedisModule_CloseKey(key);

  return idx;
}

int Index_Delete(RedisModuleCtx *ctx, const char *graphName, Graph *g, const char *label, char *prop) {
  LabelStore *store = LabelStore_Get(ctx, STORE_NODE, graphName, label);
  assert(store);

  size_t *index_id = TrieMap_Find(store->properties, prop, strlen(prop));

  if (index_id == NULL) return INDEX_FAIL;

  // Open key with write access
  RedisModuleKey *key = _index_LookupKey(ctx, graphName, *index_id, true);

  if (RedisModule_ModuleTypeGetType(key) != IndexRedisModuleType) {
    // Reply with error if this key does not exist or does not correspond to an index object
    RedisModule_CloseKey(key);
    return INDEX_FAIL;
  }

  RedisModule_DeleteKey(key);
  RedisModule_CloseKey(key);

  // NULL out the dropped index in its label schema
  LabelStore_AssignValue(store, prop, NULL);

  return INDEX_OK;
}

void initializeSkiplists(Index *index) {
  index->string_sl = skiplistCreate(compareStrings, compareNodes, cloneKey, freeKey);
  index->numeric_sl = skiplistCreate(compareNumerics, compareNodes, cloneKey, freeKey);
}

/* buildIndex allocates an Index object and populates it with a label-property pair
 * by traversing a label matrix with a TuplesIter. */
Index* buildIndex(Graph *g, const GrB_Matrix label_matrix, const char *label, const char *prop_str) {
  Index *index = malloc(sizeof(Index));

  index->label = strdup(label);
  index->property = strdup(prop_str);

  initializeSkiplists(index);

  TuplesIter *it = TuplesIter_new(label_matrix);

  Node *node;
  EntityProperty *prop;

  skiplist *sl;
  NodeID node_id;
  int found;
  int prop_index = 0;
  while(TuplesIter_next(it, NULL, &node_id) != TuplesIter_DEPLETED) {
    node = Graph_GetNode(g, node_id);
    // If the sought property is at a different offset than it occupied in the previous node,
    // then seek and update
    if (strcmp(prop_str, node->properties[prop_index].name)) {
      found = 0;
      for (int i = 0; i < node->prop_count; i ++) {
        prop = node->properties + i;
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

    prop = node->properties + prop_index;
    // This value will be cloned within the skiplistInsert routine if necessary
    SIValue *key = &prop->value;

    assert(key->type == T_STRING || key->type & SI_NUMERIC);
    sl = (key->type & SI_NUMERIC) ? index->numeric_sl: index->string_sl;
    skiplistInsert(sl, key, node_id);
  }

  TuplesIter_free(it);

  return index;
}

// Create and populate index for specified property
// (This function will create separate string and numeric indices if property has mixed types)
int Index_Create(RedisModuleCtx *ctx, const char *graphName, Graph *g, const char *label, char *prop_str) {
  LabelStore *store = LabelStore_Get(ctx, STORE_NODE, graphName, label);
  assert(store);

  size_t *index_id = TrieMap_Find(store->properties, prop_str, strlen(prop_str));
  if (index_id) return INDEX_FAIL; // This property is already indexed.

  // Create a unique index ID and attach it as a value to the property in the label schema.
  // TODO This will only update the passed label; multi-label support will require
  // iterating over labels and updating all schemas.
  index_id = malloc(sizeof(size_t));
  *index_id = Graph_AddIndexID(g);
  LabelStore_AssignValue(store, prop_str, index_id);

  // Open key with write access
  RedisModuleKey *key = _index_LookupKey(ctx, graphName, *index_id, true);
  // Do nothing if this key already exists
  if (RedisModule_ModuleTypeGetType(key) != REDISMODULE_KEYTYPE_EMPTY) {
    RedisModule_CloseKey(key);
    return INDEX_FAIL;
  }

  const GrB_Matrix label_matrix = Graph_GetLabel(g, store->id);

  Index *idx = buildIndex(g, label_matrix, label, prop_str);
  idx->id = *index_id;

  RedisModule_ModuleTypeSetValue(key, IndexRedisModuleType, idx);
  RedisModule_CloseKey(key);

  return INDEX_OK;
}

// TODO separate skiplist logic?
void Index_DeleteNode(Index *idx, NodeID node, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistDelete(sl, val, &node);
}

void Index_InsertNode(Index *idx, NodeID node, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistInsert(sl, val, node);
}

void Index_UpdateNodeValue(Index *idx, NodeID node, SIValue *oldval, SIValue *newval) {
  // It is valid for the key type to have changed in the property update,
  // so the type-checking logic in these calls is not redundant.
  Index_DeleteNode(idx, node, oldval);
  Index_InsertNode(idx, node, newval);
}

// TODO difficulty inserting into skiplist ahead of first position?
void Index_UpdateNodeID(Index *idx, NodeID prev_id, NodeID new_id, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistNode *node = skiplistFind(sl, val);
  assert(node);
  bool found = false;
  int i;
  for (i = 0; i < node->numVals; i ++) {
    if (node->vals[i] == prev_id) {
      node->vals[i] = new_id;
      found = true;
      break;
    }
  }
  // Ensure that we found the node.
  assert(found);
}

TrieMap* Indices_BuildDeletionMap(RedisModuleCtx *ctx, Graph *g, const char *graph_name, NodeID *IDs, NodeID *replacement_IDs, size_t IDCount) {
  TrieMap *index_updates = NewTrieMap();
  char *update_key;
  for (int i = 0; i < IDCount; i ++) {
    NodeID current_node = IDs[i];
    Node *delete_candidate = Graph_GetNode(g, current_node);
    // Find all matching labels for each node
    size_t *node_labels = NULL;
    size_t label_count = Graph_GetNodeLabels(g, current_node, &node_labels);
    for (int j = 0; j < label_count; j ++) {
      size_t label_id = node_labels[j];
      char *label = g->label_strings[label_id];
      LabelStore *store = LabelStore_Get(ctx, STORE_NODE, graph_name, label);

      for (int k = 0; k < delete_candidate->prop_count; k ++) {
        char *prop_name = delete_candidate->properties[k].name;
        size_t *index_id = TrieMap_Find(store->properties, prop_name, strlen(prop_name));
        // This property is not indexed; we can ignore it
        if (!index_id || index_id == TRIEMAP_NOTFOUND) continue;

        // TODO probably better options than this for forming a key
        asprintf(&update_key, "%zu", *index_id);

        // Retrieve the Vector of pending updates to this index
        Vector *to_update = TrieMap_Find(index_updates, update_key, strlen(update_key));
        // Skip unindexed properties
        if (to_update == NULL) continue;

        if (to_update == TRIEMAP_NOTFOUND) {
          // We have not encountered this label-property pair yet, create a new vector.
          // TODO can use vector, arr.h, or just stack array
          to_update = NewVector(NodeID, 2);
          // Add trie with vector of IDs as value if label-property pair is indexed
          TrieMap_Add(index_updates, update_key, strlen(update_key), to_update, NULL);
        }
        // Retrieved or built a vector for an index update - append current node
        // can also push index of property within node if desired, space-computation tradeoff
        Vector_Push(to_update, current_node);
        if (replacement_IDs) Vector_Push(to_update, replacement_IDs[i]);

        free(update_key);
      }
    }
    free(node_labels);
  }
  if (index_updates->cardinality == 0) {
    TrieMap_Free(index_updates, NULL);
    return NULL;
  }
  return index_updates;
}

void Indices_DeleteNodes(RedisModuleCtx *ctx, Graph *g, const char *graph_name, TrieMap *delete_map) {
  char *ptr;
  tm_len_t len;
  void *v;
  size_t index_id;
  Index *idx = NULL;
  NodeID node_id;

  TrieMapIterator *it = TrieMap_Iterate(delete_map, "", 0);
  while(TrieMapIterator_Next(it, &ptr, &len, &v)) {
    Vector *node_ids = v;
    sscanf(ptr, "%zu", &index_id);
    // We've switched to a new index
    RedisModuleKey *key = _index_LookupKey(ctx, graph_name, index_id, true);
    assert(RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType);
    idx = RedisModule_ModuleTypeGetValue(key);
    // Unnecessary, but keep for now
    assert(idx);

    while(Vector_Pop(node_ids, &node_id)) {
      Node *current_node = Graph_GetNode(g, node_id);
      for (int i = 0; i < current_node->prop_count; i ++) {
        if (!strcmp(idx->property, current_node->properties[i].name)) {
          Index_DeleteNode(idx, node_id, &current_node->properties[i].value);
          break;
        }
      }
    }
    RedisModule_CloseKey(key);
  }
}

void Indices_UpdateNodeIDs(RedisModuleCtx *ctx, Graph *g, const char *graph_name, TrieMap *update_map) {
  NodeID src;
  NodeID dest;
  char *ptr;
  tm_len_t len;
  void *v;
  size_t index_id;
  Index *idx = NULL;

  TrieMapIterator *it = TrieMap_Iterate(update_map, "", 0);
  while(TrieMapIterator_Next(it, &ptr, &len, &v)) {
    Vector *node_ids = v;
    sscanf(ptr, "%zu", &index_id);
    RedisModuleKey *key = _index_LookupKey(ctx, graph_name, index_id, true);
    assert(RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType);
    idx = RedisModule_ModuleTypeGetValue(key);
    // Unnecessary, but keep for now
    assert(idx);

    int count = Vector_Size(node_ids);
    for (int i = 0; i < count; i += 2) {
      Vector_Pop(node_ids, &dest);
      Vector_Pop(node_ids, &src);

      bool prop_found = false;
      Node *current_node = Graph_GetNode(g, src);
      for (int j = 0; j < current_node->prop_count; j ++) {
        if (!strcmp(idx->property, current_node->properties[j].name)) {
          Index_UpdateNodeID(idx, src, dest, &current_node->properties[j].value);
          prop_found = true;
          break;
        }
      }
      assert(prop_found);
    }
    RedisModule_CloseKey(key);
  }
}

/* Invoked from the op_create context */
void Indices_AddNode(RedisModuleCtx *ctx, LabelStore *store, const char *graphName, Node *node) {
  size_t *index_id;
  EntityProperty prop;
  Index *idx;
  for (int i = 0; i < node->prop_count; i ++) {
    prop = node->properties[i];
    index_id = TrieMap_Find(store->properties, prop.name, strlen(prop.name));
    if (!index_id || index_id == TRIEMAP_NOTFOUND) continue;

    // Retrieve index with write access
    RedisModuleKey *key = _index_LookupKey(ctx, graphName, *index_id, true);
    assert(RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType);
    idx = RedisModule_ModuleTypeGetValue(key);
    Index_InsertNode(idx, node->id, &prop.value);
  }
}

/* Invoked from the op_update context */
// void Indices_UpdateNode(RedisModuleCtx *ctx, const char *graphName, NodeID id, SIValue *oldval, SIValue *newval) {
void Indices_UpdateNode(RedisModuleCtx *ctx, LabelStore *store, const char *graphName,
                        NodeID id, EntityProperty *oldval, SIValue *newval) {
  size_t *index_id = TrieMap_Find(store->properties, oldval->name, strlen(oldval->name));
  // Label-property pair is not indexed
  if (!index_id || index_id == TRIEMAP_NOTFOUND) return;

  // Retrieve index with write access
  RedisModuleKey *key = _index_LookupKey(ctx, graphName, *index_id, true);
  assert(RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType);
  Index *idx = RedisModule_ModuleTypeGetValue(key);
  RedisModule_CloseKey(key);

  // Replace old value with new
  Index_UpdateNodeValue(idx, id, &oldval->value, newval);
}

/* Output text for EXPLAIN calls */
const char* Index_OpPrint(AST_IndexNode *indexNode) {
  switch(indexNode->operation) {
    case CREATE_INDEX:
      return "Create Index";
    default:
      return "Drop Index";
  }
}

/* Generate an iterator with no lower or upper bound. */
IndexIter* IndexIter_Create(Index *idx, SIType type) {
  skiplist *sl = type == T_STRING ? idx->string_sl : idx->numeric_sl;
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
  free(idx->label);
  free(idx->property);
  free(idx);
}

