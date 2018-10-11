#include "graphcontext.h"
#include "graphcontext_type.h"

RedisModuleKey* GraphContext_Key(RedisModuleCtx *ctx, const char *graph_name) {
  int keylen = strlen(graph_name) + 4;
  char strKey[keylen + 1];
  sprintf(strKey, "%s_ctx", graph_name);

  RedisModuleString *rm_graphctx = RedisModule_CreateString(ctx, strKey, keylen);
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rm_graphctx, REDISMODULE_WRITE);
  RedisModule_FreeString(ctx, rm_graphctx);

  return key;
}

GraphContext* GraphContext_New(RedisModuleCtx *ctx, RedisModuleString *rm_name, const char *graph_name) {
  if (GraphContext_Get(ctx, rm_name, graph_name) != NULL) return NULL;

  Graph *g = Graph_Get(ctx, rm_name);

  GraphContext *gc = malloc(sizeof(GraphContext));
  gc->ctx = ctx;
  gc->g = g;
  gc->graph_name = strdup(graph_name);
  gc->node_stores = calloc(GRAPH_DEFAULT_LABEL_CAP, sizeof(LabelStore*));
  gc->relation_stores = calloc(GRAPH_DEFAULT_RELATION_CAP, sizeof(LabelStore*));
  gc->indices = NULL;

  gc->relation_cap = GRAPH_DEFAULT_RELATION_CAP;
  gc->relation_count = 0;
  gc->node_cap = GRAPH_DEFAULT_LABEL_CAP;
  gc->node_count = 0;
  gc->index_count = 0;

  // LabelStore_Get will construct allstores if they don't already exist,
  // though I'd like to refactor its logic somewhat more generally.
  gc->node_allstore = LabelStore_Get(ctx, STORE_NODE, graph_name, NULL);
  gc->relation_allstore = LabelStore_Get(ctx, STORE_EDGE, graph_name, NULL);

  // TODO is there a way to populate store arrays here?

  RedisModuleKey *key = GraphContext_Key(ctx, graph_name);
  assert(RedisModule_ModuleTypeGetType(key) == REDISMODULE_KEYTYPE_EMPTY);
  RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);
  RedisModule_CloseKey(key);

  return gc;
}

GraphContext* GraphContext_Get(RedisModuleCtx *ctx, RedisModuleString *rs_graph_name, const char *graph_name) {
  Graph *g = Graph_Get(ctx, rs_graph_name);
  if (!g) return NULL;

  RedisModuleKey *key = GraphContext_Key(ctx, graph_name);
  if (RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) return NULL;

  GraphContext *gc = RedisModule_ModuleTypeGetValue(key);
  RedisModule_CloseKey(key);

  gc->ctx = ctx;
  gc->g = g;
  gc->node_allstore = LabelStore_Get(ctx, STORE_NODE, graph_name, NULL);
  gc->relation_allstore = LabelStore_Get(ctx, STORE_EDGE, graph_name, NULL);

  // Retrieve all label stores?
  // Can iterate over all label strings or serialize them within GraphContext value

  return gc;
}

// TODO next two functions are nearly identical; maybe consolidate
LabelStore* GraphContext_AddNode(GraphContext *gc, const char *label) {
  if(gc->node_count == gc->node_cap) {
    gc->node_cap += 4;
    gc->node_stores = realloc(gc->node_stores, gc->node_cap * sizeof(LabelStore*));
    memset(gc->node_stores + gc->node_count, 0, 4 * sizeof(LabelStore*));
  }
  // Graph_AddLabel(gc->g);
  LabelStore *store = LabelStore_New(gc->ctx, STORE_NODE, gc->graph_name, label, gc->node_count);
  gc->node_stores[gc->node_count] = store;
  gc->node_count++;

  return store;
}

LabelStore* GraphContext_AddRelation(GraphContext *gc, const char *label) {
  if(gc->relation_count == gc->relation_cap) {
    gc->relation_cap += 4;
    gc->relation_stores = realloc(gc->relation_stores, gc->relation_cap * sizeof(LabelStore*));
    memset(gc->relation_stores + gc->relation_count, 0, 4 * sizeof(LabelStore*));
  }

  LabelStore *store = LabelStore_New(gc->ctx, STORE_EDGE, gc->graph_name, label, gc->relation_count);
  gc->relation_stores[gc->relation_count] = store;
  gc->relation_count++;

  return store;
}

int GraphContext_GetLabelID(const GraphContext *gc, const char *label, LabelStoreType t) {
  LabelStore **labels;
  size_t count;
  if (t == STORE_NODE) {
    labels = gc->node_stores;
    count = gc->node_count;
  } else {
    labels = gc->relation_stores;
    count = gc->relation_count;
  }

  for (int i = 0; i < count; i ++) {
    if (labels[i] && !strcmp(label, labels[i]->label)) return i;
  }
  return GRAPH_NO_LABEL; // equivalent to GRAPH_NO_RELATION
}

Index* GraphContext_GetIndex(GraphContext *gc, const char *label, const char *property) {
  Index *idx;
  for (int i = 0; i < gc->index_count; i ++) {
    idx = gc->indices[i];
    if (!strcmp(label, idx->label) && !strcmp(property, idx->property)) return idx;
  }
  return NULL;
}

bool GraphContext_HasIndices(GraphContext *gc) {
  return gc->index_count > 0;
}

void GraphContext_AddIndex(GraphContext *gc, Index *idx) {
  gc->index_count++;
  if (!gc->indices) {
    gc->indices = malloc(gc->index_count * sizeof(Index*));
  } else {
    gc->indices = realloc(gc->indices, gc->index_count * sizeof(Index*));
  }
  gc->indices[gc->index_count - 1] = idx;
}

LabelStore* GraphContext_AllStore(const GraphContext *gc, LabelStoreType t) {
  if (t == STORE_NODE) return gc->node_allstore;
  return gc->relation_allstore;
}

LabelStore* GraphContext_GetStore(const GraphContext *gc, const char *label, LabelStoreType t) {
  LabelStore **stores;
  size_t count;
  if (t == STORE_NODE) {
    stores = gc->node_stores;
    count = gc->node_count;
  } else {
    stores = gc->relation_stores;
    count = gc->relation_count;
  }

  // Check cached stores
  int id = GraphContext_GetLabelID(gc, label, t);
  if (id != GRAPH_NO_LABEL) return stores[id];

  // If store is not cached, retrieve from keyspace
  LabelStore *store = LabelStore_Get(gc->ctx, t, gc->graph_name, label);
  if (!store) return NULL;

  assert(store->id < count);
  gc->relation_stores[store->id] = store;

  return store;
}

void GraphContext_Free(GraphContext *gc) {
  // TODO if the GraphContext is loaded from rdb, it does not own graph_name and should not free it.
  free(gc->graph_name);
  for (int i = 0; i < gc->index_count; i ++) {
    Index_Free(gc->indices[i]);
  }
  if (gc->indices) free(gc->indices);
  free(gc->relation_stores);
  free(gc->node_stores);
  free(gc);
}

