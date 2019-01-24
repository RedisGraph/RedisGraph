#include <sys/param.h>
#include "graphcontext.h"
#include "serializers/graphcontext_type.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../redismodule.h"

extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

GraphContext* GraphContext_New(RedisModuleCtx *ctx, const char *graphname,
                               size_t node_cap, size_t edge_cap) {
  GraphContext *gc = NULL;

  // Create key for GraphContext from the unmodified string provided by the user
  RedisModuleString *rs_name = RedisModule_CreateString(ctx, graphname, strlen(graphname));
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_WRITE);
  if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY) {
    goto cleanup;
  }

  gc = rm_malloc(sizeof(GraphContext));

  // Initialize the graph's matrices and datablock storage
  gc->g = Graph_New(node_cap, edge_cap);

  gc->graph_name = rm_strdup(graphname);
  // Allocate the default space for stores and indices
  gc->node_stores = array_new(LabelStore*, GRAPH_DEFAULT_LABEL_CAP);
  gc->relation_stores = array_new(LabelStore*, GRAPH_DEFAULT_RELATION_TYPE_CAP);
  gc->indices = array_new(Index*, DEFAULT_INDEX_CAP);

  // Initialize the generic label and relation stores
  gc->node_allstore = LabelStore_New("ALL", GRAPH_NO_LABEL);
  gc->relation_allstore = LabelStore_New("ALL", GRAPH_NO_RELATION);

  pthread_setspecific(_tlsGCKey, gc);

  // Set and close GraphContext key in Redis keyspace
  RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);

cleanup:
  RedisModule_CloseKey(key);
  RedisModule_FreeString(ctx, rs_name);
  return gc;
}

GraphContext* GraphContext_Retrieve(RedisModuleCtx *ctx, const char *graphname) {
  GraphContext *gc = NULL;
  RedisModuleString *rs_name = RedisModule_CreateString(ctx, graphname, strlen(graphname));
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_READ);
  if (RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
    goto cleanup;
  }
  gc = RedisModule_ModuleTypeGetValue(key);

  // Force GraphBLAS updates and resize matrices to node count by default
  Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

  pthread_setspecific(_tlsGCKey, gc);

cleanup:
  RedisModule_FreeString(ctx, rs_name);
  RedisModule_CloseKey(key);
  return gc;
}

GraphContext* GraphContext_GetFromLTS() {
  GraphContext* gc = pthread_getspecific(_tlsGCKey);
  assert(gc);
  return gc;
}

//------------------------------------------------------------------------------
// LabelStore API
//------------------------------------------------------------------------------
// Find the ID associated with a label for store and matrix access
int _GraphContext_GetLabelID(const GraphContext *gc, const char *label, LabelStoreType t) {
  // Choose the appropriate store array given the entity type
  LabelStore **labels = (t == STORE_NODE) ? gc->node_stores : gc->relation_stores;

  // TODO optimize lookup
  for (uint32_t i = 0; i < array_len(labels); i ++) {
    if (!strcmp(label, labels[i]->label)) return i;
  }
  return GRAPH_NO_LABEL; // equivalent to GRAPH_NO_RELATION
}

LabelStore* GraphContext_AllStore(const GraphContext *gc, LabelStoreType t) {
  if (t == STORE_NODE) return gc->node_allstore;
  return gc->relation_allstore;
}

LabelStore* GraphContext_GetStore(const GraphContext *gc, const char *label, LabelStoreType t) {
  LabelStore **stores = (t == STORE_NODE) ? gc->node_stores : gc->relation_stores;

  int id = _GraphContext_GetLabelID(gc, label, t);
  if (id == GRAPH_NO_LABEL) return NULL;

  return stores[id];
}

// The next two functions are identical save for whether their target is a label or relation type,
// but modify enough variables to make consolidating them unwieldy.
LabelStore* GraphContext_AddLabel(GraphContext *gc, const char *label) {
  Graph_AddLabel(gc->g);
  LabelStore *store = LabelStore_New(label, array_len(gc->node_stores));
  gc->node_stores = array_append(gc->node_stores, store);

  return store;
}

LabelStore* GraphContext_AddRelationType(GraphContext *gc, const char *label) {
  Graph_AddRelationType(gc->g);
  LabelStore *store = LabelStore_New(label, array_len(gc->relation_stores));
  gc->relation_stores = array_append(gc->relation_stores, store);

  return store;
}

//------------------------------------------------------------------------------
// Index API
//------------------------------------------------------------------------------
Index* _GraphContext_GetIndexFromStore(LabelStore *store, char *property) {
  Index *idx = LabelStore_RetrieveValue(store, property);
  if (idx == TRIEMAP_NOTFOUND || idx == NULL) {
    // Property does not exist or was not indexed.
    return NULL;
  }
  return idx;
}

bool GraphContext_HasIndices(GraphContext *gc) {
  return array_len(gc->indices) > 0;
}

Index* GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *property) {
  // Retrieve the store for this label
  LabelStore *store = GraphContext_GetStore(gc, label, STORE_NODE);
  if (store == NULL) return NULL;
  Index *idx = _GraphContext_GetIndexFromStore(store, (char*)property);
  return idx;
}

int GraphContext_AddIndex(GraphContext *gc, const char *label, const char *property) {
  // Retrieve the store for this label
  LabelStore *store = GraphContext_GetStore(gc, label, STORE_NODE);
  if (store == NULL) return INDEX_FAIL;

  // Verify that property exists and is not already indexed.
  void *property_lookup = LabelStore_RetrieveValue(store, (char*)property);
  if (property_lookup == TRIEMAP_NOTFOUND || property_lookup != NULL) {
    return INDEX_FAIL;
  }

  // Populate an index for the label-property pair using the Graph interfaces.
  Index *idx = Index_Create(gc->g, store->id, label, property);
  gc->indices = array_append(gc->indices, idx);

  // Associate the new index with the property in the label store.
  LabelStore_AssignValue(store, (char*)property, (void*)idx);

  return INDEX_OK;
}

int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *property) {
  // Retrieve the store for this label
  LabelStore *store = GraphContext_GetStore(gc, label, STORE_NODE);
  if (store == NULL) return INDEX_FAIL;

  Index *idx = _GraphContext_GetIndexFromStore(store, (char*)property);
  // Property does not exist or was not indexed.
  if (idx == NULL) return INDEX_FAIL;

  // Remove the index association from the label store
  LabelStore_AssignValue(store, (char*)property, NULL);
  // Index *idx = gc->indices[offset];

  // Pop the last stored index
  Index *last_idx = array_pop(gc->indices);
  if (idx != last_idx) {
    // If the index being deleted is not the last, swap the last into the newly-emptied position
    uint32_t pos;
    // Find the position of the index in the gc->indices array
    for (pos = 0; pos < array_len(gc->indices); pos ++) {
      if (gc->indices[pos] == idx) {
        gc->indices[pos] = last_idx;
        break;
      }
    }
    // Fail if the index was somehow not in the indices array.
    assert(pos < array_len(gc->indices));
  }

  // Free the index
  Index_Free(idx);

  return INDEX_OK;
}

// Add references to a node to all indices built upon its properties
void GraphContext_AddNodeToIndices(GraphContext *gc, LabelStore *store, Node *n) {
  if (store && GraphContext_HasIndices(gc)) {
    EntityProperty *props = ENTITY_PROPS(n);
    for (int j = 0; j < ENTITY_PROP_COUNT(n); j ++) {
      // If a property is indexed, update it to include the given node 
      Index *idx = _GraphContext_GetIndexFromStore(store, props[j].name);
      if (idx) Index_InsertNode(idx, n->entity->id, &props[j].value);
    }
  }
}

// Delete all references to a node from any indices built upon its properties
void GraphContext_DeleteNodeFromIndices(GraphContext *gc, LabelStore *store, Node *n) {
  if (!GraphContext_HasIndices(gc)) return;

  if (store == NULL) {
    if (n->label) {
      // Node will have a label string if one was specified in the query MATCH clause
      store = GraphContext_GetStore(gc, n->label, STORE_NODE);
    } else {
      // Otherwise, look up the offset of the matching label (if any)
      int store_id = Graph_GetNodeLabel(gc->g, n->entity->id);
      // Do nothing if node had no label
      if (store_id == GRAPH_NO_LABEL) return;
      store = gc->node_stores[store_id];
    }
  }

  // Update any indices this entity is represented in
  EntityProperty *props = ENTITY_PROPS(n);
  for (int j = 0; j < ENTITY_PROP_COUNT(n); j ++) {
    Index *idx = LabelStore_RetrieveValue(store, props[j].name);
    if (idx != TRIEMAP_NOTFOUND && idx != NULL) {
      Index_DeleteNode(idx, n->entity->id, &props[j].value);
    }
  }
}

// If the given property is indexed, update node reference within index to point to correct value
void GraphContext_UpdateNodeIndices(GraphContext *gc, LabelStore *store, NodeID id, EntityProperty *prop, SIValue *newval) {
  if (!GraphContext_HasIndices(gc)) return;

  // If the query did not specify a label, retrieve the correct label store now
  if (store == NULL) {
    int store_id = Graph_GetNodeLabel(gc->g, id);
    if (store_id == GRAPH_NO_LABEL) return;
    store = gc->node_stores[store_id];
  }

  Index *idx = _GraphContext_GetIndexFromStore(store, prop->name);
  if (idx == NULL) return;

  // Delete the original node and value from the index
  Index_DeleteNode(idx, id, &prop->value);
  // Re-insert the node with its update value
  Index_InsertNode(idx, id, newval);
}

//------------------------------------------------------------------------------
// Free routine
//------------------------------------------------------------------------------

// Free all data associated with graph
void GraphContext_Free(GraphContext *gc) {
  Graph_Free(gc->g);
  rm_free(gc->graph_name);

  // Free generic stores
  LabelStore_Free(gc->node_allstore);
  LabelStore_Free(gc->relation_allstore);

  // Free all node stores
  if(gc->node_stores) {
    for (uint32_t i = 0; i < array_len(gc->node_stores); i ++) {
      LabelStore_Free(gc->node_stores[i]);
    }
    array_free(gc->node_stores);
  }

  // Free all relation stores
  if(gc->relation_stores) {
    for (uint32_t i = 0; i < array_len(gc->relation_stores); i ++) {
      LabelStore_Free(gc->relation_stores[i]);
    }
    array_free(gc->relation_stores);
  }

  // Free all indices
  if(gc->indices) {
    for (uint32_t i = 0; i < array_len(gc->indices); i ++) {
      Index_Free(gc->indices[i]);
    }
    array_free(gc->indices);
  }

  rm_free(gc);
}
