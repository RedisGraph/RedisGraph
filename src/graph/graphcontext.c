#include "graphcontext.h"
#include "graphcontext_type.h"
// TODO needed for the key-value creation logic for the Graph
extern RedisModuleType *GraphRedisModuleType;

// TODO should be unnecessary
int _GraphContext_IndexOffset(GraphContext *gc, const char *label, const char *property) {
  Index *idx;
  for (int i = 0; i < gc->index_count; i ++) {
    idx = gc->indices[i];
    if (!strcmp(label, idx->label) && !strcmp(property, idx->property)) return i;
  }
  return -1;
}

RedisModuleKey* GraphContext_Key(RedisModuleCtx *ctx, const char *graph_name) {
  int keylen = strlen(graph_name) + 4;
  char strKey[keylen + 1];
  sprintf(strKey, "%s_ctx", graph_name);

  RedisModuleString *rm_graphctx = RedisModule_CreateString(ctx, strKey, keylen);
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rm_graphctx, REDISMODULE_WRITE);
  RedisModule_FreeString(ctx, rm_graphctx);

  return key;
}

GraphContext* GraphContext_New(RedisModuleCtx *ctx, RedisModuleString *rs_name) {
  // Create key for GraphContext
  const char *graph_name = RedisModule_StringPtrLen(rs_name, NULL);
  RedisModuleKey *key = GraphContext_Key(ctx, graph_name);
  if (RedisModule_ModuleTypeGetType(key) != REDISMODULE_KEYTYPE_EMPTY) {
    // Return NULL if key already exists
    RedisModule_CloseKey(key);
    return NULL;
  }

  GraphContext *gc = malloc(sizeof(GraphContext));
  gc->relation_cap = GRAPH_DEFAULT_RELATION_CAP;
  gc->relation_count = 0;
  gc->label_cap = GRAPH_DEFAULT_LABEL_CAP;
  gc->label_count = 0;
  gc->index_cap = 4;
  gc->index_count = 0;

  gc->graph_name = strdup(graph_name);
  gc->node_stores = malloc(gc->label_cap * sizeof(LabelStore*));
  gc->relation_stores = malloc(gc->relation_cap * sizeof(LabelStore*));
  gc->indices = malloc(gc->index_cap * sizeof(Index*));

  gc->node_allstore = LabelStore_New("ALL", GRAPH_NO_LABEL);
  gc->relation_allstore = LabelStore_New("ALL", GRAPH_NO_RELATION);

  // Set and close GraphContext in keyspace
  RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);
  RedisModule_CloseKey(key);

  gc->g = Graph_New(GRAPH_DEFAULT_NODE_CAP);
  // Add new Graph to keyspace
  // TODO this separation feels rather clunky
  key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_WRITE);
  assert(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY);
  RedisModule_ModuleTypeSetValue(key, GraphRedisModuleType, gc->g);
  RedisModule_CloseKey(key);

  return gc;
}

GraphContext* GraphContext_Get(RedisModuleCtx *ctx, RedisModuleString *rs_name) {
  const char *graph_name = RedisModule_StringPtrLen(rs_name, NULL);
  RedisModuleKey *key = GraphContext_Key(ctx, graph_name);
  if (RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) return NULL;

  GraphContext *gc = RedisModule_ModuleTypeGetValue(key);
  RedisModule_CloseKey(key);

  gc->g = Graph_Get(ctx, rs_name);

  return gc;
}

// TODO next two functions are nearly identical; maybe consolidate
LabelStore* GraphContext_AddLabel(GraphContext *gc, const char *label) {
  if(gc->label_count == gc->label_cap) {
    gc->label_cap += 4;
    // Add space for additional label stores.
    gc->node_stores = realloc(gc->node_stores, gc->label_cap * sizeof(LabelStore*));
    // Add space for additional label matrices.
    // TODO is this too much of an abstraction breakage?
    gc->g->_labels = realloc(gc->g->_labels, gc->label_cap * sizeof(GrB_Matrix));

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
    gc->relation_stores = realloc(gc->relation_stores, gc->relation_cap * sizeof(LabelStore*));
    gc->g->_relations = realloc(gc->g->_relations, gc->relation_cap * sizeof(GrB_Matrix));
  }
  Graph_AddRelationType(gc->g);

  LabelStore *store = LabelStore_New(label, gc->relation_count);
  gc->relation_stores[gc->relation_count] = store;
  gc->relation_count++;

  return store;
}

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

  for (int i = 0; i < count; i ++) {
    if (!strcmp(label, labels[i]->label)) return i;
  }
  return GRAPH_NO_LABEL; // equivalent to GRAPH_NO_RELATION
}

bool GraphContext_HasIndices(GraphContext *gc) {
  return gc->index_count > 0;
}

Index* GraphContext_GetIndex(GraphContext *gc, const char *label, const char *property) {
  int offset = _GraphContext_IndexOffset(gc, label, property);
  if (offset < 0) return INDEX_FAIL;

  return gc->indices[offset];
}

void GraphContext_AddIndex(GraphContext *gc, const char *label, const char *property) {
  if(gc->index_count == gc->index_cap) {
    gc->index_cap += 4;
    gc->indices = realloc(gc->indices, gc->index_cap * sizeof(Index*));
  }

  LabelStore *store = GraphContext_GetStore(gc, label, STORE_NODE);
  const GrB_Matrix label_matrix = Graph_GetLabel(gc->g, store->id);
  TuplesIter *it = TuplesIter_new(label_matrix);
  Index *idx = Index_Create(gc->g->nodes, it, label, property);
  TuplesIter_free(it);
  gc->indices[gc->index_count] = idx;
  gc->index_count++;
}

int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *property) {
  int offset = _GraphContext_IndexOffset(gc, label, property);
  if (offset < 0) return INDEX_FAIL;

  Index *idx = gc->indices[offset];
  Index_Free(idx);
  gc->index_count --;
  gc->indices[offset] = gc->indices[gc->index_count];

  return INDEX_OK;
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

void GraphContext_Free(GraphContext *gc) {
  free(gc->graph_name);
  for (int i = 0; i < gc->index_count; i ++) {
    Index_Free(gc->indices[i]);
  }
  free(gc->indices);
  free(gc->relation_stores);
  free(gc->node_stores);
  free(gc);
}

