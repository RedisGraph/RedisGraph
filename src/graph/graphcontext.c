#include <sys/param.h>
#include "graphcontext.h"
#include "serializers/graphcontext_type.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

GraphContext* GraphContext_New(RedisModuleCtx *ctx, RedisModuleString *rs_name,
                               size_t node_cap, size_t edge_cap) {
  // Create key for GraphContext from the unmodified string provided by the user
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_WRITE);
  if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY) {
    // Return NULL if key already exists
    RedisModule_CloseKey(key);
    return NULL;
  }

  GraphContext *gc = rm_malloc(sizeof(GraphContext));

  // Initialize the graph's matrices and datablock storage
  gc->g = Graph_New(node_cap, edge_cap);

  gc->graph_name = rm_strdup(RedisModule_StringPtrLen(rs_name, NULL));
  // Allocate the default space for stores and indices
  gc->node_stores = array_new(LabelStore*, GRAPH_DEFAULT_LABEL_CAP);
  gc->relation_stores = array_new(LabelStore*, GRAPH_DEFAULT_RELATION_TYPE_CAP);
  gc->indices = array_new(Index*, DEFAULT_INDEX_CAP);

  // Initialize the generic label and relation stores
  gc->node_allstore = LabelStore_New("ALL", GRAPH_NO_LABEL);
  gc->relation_allstore = LabelStore_New("ALL", GRAPH_NO_RELATION);

  // Set and close GraphContext key in Redis keyspace
  RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);
  RedisModule_CloseKey(key);

  return gc;
}

GraphContext* GraphContext_Retrieve(RedisModuleCtx *ctx, RedisModuleString *rs_name) {
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_READ);
  if (RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
    RedisModule_CloseKey(key);
    return NULL;
  }
  GraphContext *gc = RedisModule_ModuleTypeGetValue(key);
  RedisModule_CloseKey(key);

  // Force GraphBLAS updates and resize matrices to node count by default
  Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

  return gc;
}

//------------------------------------------------------------------------------
// LabelStore API
//------------------------------------------------------------------------------
int GraphContext_GetLabelID(const GraphContext *gc, const char *label, LabelStoreType t) {
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

  int id = GraphContext_GetLabelID(gc, label, t);
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
bool GraphContext_HasIndices(GraphContext *gc) {
  return array_len(gc->indices) > 0;
}

Index* GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *property) {
  // Find the ID of the specified label
  int label_id = GraphContext_GetLabelID(gc, label, STORE_NODE);
  if (label_id < 0 ) return INDEX_FAIL;

  LabelStore *store = gc->node_stores[label_id];

  Index *idx = LabelStore_RetrieveValue(store, (char*)property);
  if (idx == TRIEMAP_NOTFOUND || idx == NULL) {
    // Property does not exist or was not indexed.
    return INDEX_FAIL;
  }
  return idx;
}

int GraphContext_AddIndex(GraphContext *gc, const char *label, const char *property) {
  // Find the ID of the specified label
  int label_id = GraphContext_GetLabelID(gc, label, STORE_NODE);
  if (label_id < 0 ) return INDEX_FAIL;

  LabelStore *store = gc->node_stores[label_id];

  // Triemaps (for some reason) don't specify const.
  void *property_lookup = LabelStore_RetrieveValue(store, (char*)property);

  if (property_lookup == TRIEMAP_NOTFOUND || property_lookup != NULL) {
    // Property does not exist on this label or property is already indexed.
    return INDEX_FAIL;
  }

  // Populate an index for the label-property pair using the Graph interfaces.
  Index *idx = Index_Create(gc->g, label_id, label, property);
  gc->indices = array_append(gc->indices, idx);

  // Associate the new index with the property in the label store.
  LabelStore_AssignValue(store, (char*)property, (void*)idx);

  return INDEX_OK;
}

int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *property) {
  // Find the ID of the specified label
  int label_id = GraphContext_GetLabelID(gc, label, STORE_NODE);
  if (label_id < 0 ) return INDEX_FAIL;

  LabelStore *store = gc->node_stores[label_id];

  Index *idx = LabelStore_RetrieveValue(store, (char*)property);
  if (idx == TRIEMAP_NOTFOUND || idx == NULL) {
    // Property does not exist or was not indexed.
    return INDEX_FAIL;
  }

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
  }

  // Free the index
  Index_Free(idx);

  return INDEX_OK;
}

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

