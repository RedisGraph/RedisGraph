#include "index_updates.h"

void _index_DeleteNode(Index *idx, NodeID node, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistDelete(sl, val, &node);
}

void _index_InsertNode(Index *idx, NodeID node, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistInsert(sl, val, node);
}

void _index_UpdateNodeID(Index *idx, NodeID prev_id, NodeID new_id, SIValue *val) {
  skiplist *sl = val->type == T_STRING ? idx->string_sl : idx->numeric_sl;
  skiplistNode *node = skiplistFind(sl, val);
  assert(node);
  int i;
  for (i = 0; i < node->numVals; i ++) {
    if (node->vals[i] == prev_id) {
      node->vals[i] = new_id;
      break;
    }
  }
  // Ensure that we found the node.
  assert(i < node->numVals);
}

TrieMap* Indices_BuildModificationMap(RedisModuleCtx *ctx, Graph *g, const char *graph_name, NodeID *IDs, NodeID *replacement_IDs, size_t IDCount) {
  TrieMap *index_updates = NewTrieMap();
  // The maximum possible number of digits in a size_t is 20 
  char update_key[21];
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
        // loop over indices to see if this label-property pair is present
        // (instead of retrieving label store)
        // This property is not indexed; we can ignore it
        if (!index_id || index_id == TRIEMAP_NOTFOUND) continue;
        // Update index here
        // create op
        // delete op (node is getting migrated) - src and dest IDs
        // delete (deleted) - lookup and remove
        // update property op

        snprintf(update_key, 21, "%zu", *index_id);

        // Retrieve the Vector of pending updates to this index
        Vector *to_update = TrieMap_Find(index_updates, update_key, strlen(update_key));
        // Skip unindexed properties
        if (to_update == NULL) continue;

        if (to_update == TRIEMAP_NOTFOUND) {
          // We have not encountered this label-property pair yet, create a new vector.
          to_update = NewVector(NodeID, 2);
          // Add trie with vector of IDs as value if label-property pair is indexed
          TrieMap_Add(index_updates, update_key, strlen(update_key), to_update, NULL);
        }
        // Retrieved or built a vector for an index update - append current node
        // can also push index of property within node if desired, space-computation tradeoff
        Vector_Push(to_update, current_node);
        if (replacement_IDs) Vector_Push(to_update, replacement_IDs[i]);
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

/* Invoked from the op_update context */
void Index_UpdateNodeValue(RedisModuleCtx *ctx, LabelStore *store, const char *graphName,
                        NodeID id, EntityProperty *oldval, SIValue *newval) {
  size_t *index_id = TrieMap_Find(store->properties, oldval->name, strlen(oldval->name));
  // Label-property pair is not indexed
  if (!index_id || index_id == TRIEMAP_NOTFOUND) return;

  // Retrieve index with write access
  RedisModuleKey *key = Index_LookupKey(ctx, graphName, *index_id, true);
  assert(RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType);
  Index *idx = RedisModule_ModuleTypeGetValue(key);
  RedisModule_CloseKey(key);

  /* Replace old property with new.
   * It is valid for the key type to have changed in the property update,
   * so the type-checking logic in these calls is not redundant. */
  _index_DeleteNode(idx, id, &oldval->value);
  _index_InsertNode(idx, id, newval);
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
    RedisModuleKey *key = Index_LookupKey(ctx, graph_name, index_id, true);
    assert(RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType);
    idx = RedisModule_ModuleTypeGetValue(key);
    RedisModule_CloseKey(key);

    while(Vector_Pop(node_ids, &node_id)) {
      Node *current_node = Graph_GetNode(g, node_id);
      for (int i = 0; i < current_node->prop_count; i ++) {
        if (!strcmp(idx->property, current_node->properties[i].name)) {
          _index_DeleteNode(idx, node_id, &current_node->properties[i].value);
          break;
        }
      }
    }
  }
}

/* Invoked from the op_delete context */
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
    RedisModuleKey *key = Index_LookupKey(ctx, graph_name, index_id, true);
    assert(RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType);
    idx = RedisModule_ModuleTypeGetValue(key);
    RedisModule_CloseKey(key);

    int count = Vector_Size(node_ids);
    for (int i = 0; i < count; i += 2) {
      Vector_Pop(node_ids, &dest);
      Vector_Pop(node_ids, &src);

      bool prop_found = false;
      Node *current_node = Graph_GetNode(g, src);
      for (int j = 0; j < current_node->prop_count; j ++) {
        if (!strcmp(idx->property, current_node->properties[j].name)) {
          _index_UpdateNodeID(idx, src, dest, &current_node->properties[j].value);
          prop_found = true;
          break;
        }
      }
      assert(prop_found);
    }
  }
}

/* Invoked from the op_create context */
void Indices_AddNodes(RedisModuleCtx *ctx, Graph *g, const char *graph_name, TrieMap *create_map) {
  char *ptr;
  tm_len_t len;
  void *v;
  size_t index_id;
  Index *idx = NULL;
  NodeID node_id;

  TrieMapIterator *it = TrieMap_Iterate(create_map, "", 0);
  while(TrieMapIterator_Next(it, &ptr, &len, &v)) {
    Vector *node_ids = v;
    sscanf(ptr, "%zu", &index_id);
    // We've switched to a new index
    RedisModuleKey *key = Index_LookupKey(ctx, graph_name, index_id, true);
    assert(RedisModule_ModuleTypeGetType(key) == IndexRedisModuleType);
    idx = RedisModule_ModuleTypeGetValue(key);
    RedisModule_CloseKey(key);

    while(Vector_Pop(node_ids, &node_id)) {
      Node *current_node = Graph_GetNode(g, node_id);
      for (int i = 0; i < current_node->prop_count; i ++) {
        if (!strcmp(idx->property, current_node->properties[i].name)) {
          _index_InsertNode(idx, node_id, &current_node->properties[i].value);
          break;
        }
      }
    }
  }
}

