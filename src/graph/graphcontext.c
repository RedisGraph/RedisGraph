#include <sys/param.h>
#include "graphcontext.h"
#include "serializers/graphcontext_type.h"
#include "../util/rmalloc.h"

//------------------------------------------------------------------------------
// Private functions
//------------------------------------------------------------------------------
int _GraphContext_IndexOffset(const GraphContext *gc, const char *label, const char *property) {
  Index *idx;
  for (int i = 0; i < gc->index_count; i ++) {
    idx = gc->indices[i];
    if (!strcmp(label, idx->label) && !strcmp(property, idx->property)) return i;
  }
  return -1;
}

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

  gc->relation_cap = GRAPH_DEFAULT_RELATION_TYPE_CAP;
  gc->relation_count = 0;
  gc->label_cap = GRAPH_DEFAULT_LABEL_CAP;
  gc->label_count = 0;
  gc->index_cap = DEFAULT_INDEX_CAP;
  gc->index_count = 0;

  // Initialize the graph's matrices and datablock storage
  gc->g = Graph_New(node_cap, edge_cap);

  gc->graph_name = rm_strdup(RedisModule_StringPtrLen(rs_name, NULL));
  // Allocate the default space for stores and indices
  gc->node_stores = rm_malloc(gc->label_cap * sizeof(LabelStore*));
  gc->relation_stores = rm_malloc(gc->relation_cap * sizeof(LabelStore*));
  gc->indices = rm_malloc(gc->index_cap * sizeof(Index*));

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
  LabelStore **labels;
  size_t count;
  if (t == STORE_NODE) {
    labels = gc->node_stores;
    count = gc->label_count;
  } else {
    labels = gc->relation_stores;
    count = gc->relation_count;
  }

  // TODO optimize lookup
  for (int i = 0; i < count; i ++) {
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
  if(gc->label_count == gc->label_cap) {
    gc->label_cap += 4;
    // Add space for additional label stores.
    gc->node_stores = rm_realloc(gc->node_stores, gc->label_cap * sizeof(LabelStore*));
    // Add space for additional label matrices.
    gc->g->labels = rm_realloc(gc->g->labels, gc->label_cap * sizeof(GrB_Matrix));

  }
  Graph_AddLabel(gc->g);
  LabelStore *store = LabelStore_New(label, gc->label_count);
  gc->node_stores[gc->label_count] = store;
  gc->label_count++;

  return store;
}

LabelStore* GraphContext_AddRelationType(GraphContext *gc, const char *label) {
  if(gc->relation_count == gc->relation_cap) {
    gc->relation_cap += 4;
    // Add space for additional relation type stores.
    gc->relation_stores = rm_realloc(gc->relation_stores, gc->relation_cap * sizeof(LabelStore*));
    // Add space for additional relation type matrices.
    gc->g->relations = rm_realloc(gc->g->relations, gc->relation_cap * sizeof(GrB_Matrix));
  }
  Graph_AddRelationType(gc->g);

  LabelStore *store = LabelStore_New(label, gc->relation_count);
  gc->relation_stores[gc->relation_count] = store;
  gc->relation_count++;

  return store;
}

//------------------------------------------------------------------------------
// Index API
//------------------------------------------------------------------------------
bool GraphContext_HasIndices(GraphContext *gc) {
  return gc->index_count > 0;
}

Index* GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *property) {
  int offset = _GraphContext_IndexOffset(gc, label, property);
  if (offset < 0) return INDEX_FAIL;

  return gc->indices[offset];
}

int GraphContext_AddIndex(GraphContext *gc, const char *label, const char *property) {
  if(gc->index_count == gc->index_cap) {
    gc->index_cap += 4;
    gc->indices = rm_realloc(gc->indices, gc->index_cap * sizeof(Index*));
  }

  // Find the ID of the specified label
  int label_id = GraphContext_GetLabelID(gc, label, STORE_NODE);
  if (label_id < 0 ) return INDEX_FAIL;

  // Populate an index for the label-property pair using the Graph interfaces.
  Index *idx = Index_Create(gc->g, label_id, label, property);
  gc->indices[gc->index_count] = idx;
  gc->index_count++;

  return INDEX_OK;
}

int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *property) {
  int offset = _GraphContext_IndexOffset(gc, label, property);
  if (offset < 0) return INDEX_FAIL;

  Index *idx = gc->indices[offset];
  Index_Free(idx);
  gc->index_count --;
  // If the deleted index was not the last, swap the last into the newly-emptied position
  if (offset < gc->index_count) gc->indices[offset] = gc->indices[gc->index_count];

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
    for (int i = 0; i < gc->label_count; i ++) {
      LabelStore_Free(gc->node_stores[i]);
    }
    rm_free(gc->node_stores);
  }

  // Free all relation stores
  if(gc->relation_stores) {
    for (int i = 0; i < gc->relation_count; i ++) {
      LabelStore_Free(gc->relation_stores[i]);
    }
    rm_free(gc->relation_stores);
  }

  // Free all indices
  if(gc->indices) {
    for (int i = 0; i < gc->index_count; i ++) {
      Index_Free(gc->indices[i]);
    }
    rm_free(gc->indices);
  }

  rm_free(gc);
}

