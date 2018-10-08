#include "graphcontext.h"
#include "graphcontext_type.h"

// TODO Change to portable thread_local definition
_Thread_local GraphContext *gc;

void GraphContext_New(RedisModuleCtx *ctx, const char *graph_name) {
  gc = malloc(sizeof(GraphContext));
  gc->ctx = ctx;
  gc->graph_name = strdup(graph_name);
  gc->node_stores = calloc(GRAPH_DEFAULT_LABEL_CAP, sizeof(LabelStore*));
  gc->relation_stores = calloc(GRAPH_DEFAULT_RELATION_CAP, sizeof(LabelStore*));

  gc->relation_cap = GRAPH_DEFAULT_RELATION_CAP;
  gc->relation_count = 0;
  gc->node_cap = GRAPH_DEFAULT_LABEL_CAP;
  gc->node_count = 0;
  gc->index_count = 0;

  // TODO I will probably make a macro for building context keys on the stack
  int keylen = strlen(graph_name) + 4;
  char strKey[keylen + 1];
  strKey[0] = '\0';
  strcat(strKey, graph_name);
  strcat(strKey, "_ctx");

  // LabelStore_Get will construct allstores if they don't already exist,
  // though I'd like to refactor its logic somewhat more generally.
  gc->node_allstore = LabelStore_Get(ctx, STORE_NODE, graph_name, NULL);
  gc->relation_allstore = LabelStore_Get(ctx, STORE_EDGE, graph_name, NULL);

  // TODO is there a way to populate store arrays here?

  RedisModuleString *rm_graphctx = RedisModule_CreateString(ctx, strKey, keylen);
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rm_graphctx, REDISMODULE_WRITE);
  assert(RedisModule_ModuleTypeGetType(key) == REDISMODULE_KEYTYPE_EMPTY);
  RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);
  RedisModule_CloseKey(key);
}

// TODO not the best name, as it doesn't return
void GraphContext_Get(RedisModuleCtx *ctx, const char *graph_name) {

  int keylen = strlen(graph_name) + 4;
  char strKey[keylen + 1];
  strKey[0] = '\0';
  strcat(strKey, graph_name);
  strcat(strKey, "_ctx");

  RedisModuleString *rm_graphctx = RedisModule_CreateString(ctx, strKey, keylen);
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rm_graphctx, REDISMODULE_WRITE);
  assert(RedisModule_ModuleTypeGetType(key) == GraphContextRedisModuleType); gc = RedisModule_ModuleTypeGetValue(key);

  gc->ctx = ctx;
  gc->node_allstore = LabelStore_Get(ctx, STORE_NODE, graph_name, NULL);
  gc->relation_allstore = LabelStore_Get(ctx, STORE_EDGE, graph_name, NULL);

  // Retrieve all label stores?
  // Can iterate over all label strings, but that sounds awful

  RedisModule_CloseKey(key);
}

// TODO next two functions are nearly identical; maybe consolidate
LabelStore* GraphContext_AddNode(const char *label) {
  if(gc->node_count == gc->node_cap) {
    gc->node_cap += 4;
    gc->node_stores = realloc(gc->node_stores, (gc->node_cap) * sizeof(LabelStore*));
    memset(gc->node_stores + gc->node_count, 0, 4 * sizeof(LabelStore*));
  }
  LabelStore *store = LabelStore_New(gc->ctx, STORE_NODE, gc->graph_name, label, gc->node_count);
  gc->node_stores[gc->node_count] = store;
  gc->node_count++;

  return store;
}

void GraphContext_AddRelation(const char *label) {
  if(gc->relation_count == gc->relation_cap) {
    gc->relation_cap += 4;
    gc->relation_stores = realloc(gc->relation_stores, (gc->relation_cap) * sizeof(LabelStore*));
    memset(gc->relation_stores + gc->relation_count, 0, 4 * sizeof(LabelStore*));
  }
  // TODO This function is pointless until bringing back the relationship label string array
  // or updating it to handle stores like AddNode
  // gc->relation_stores[gc->relation_count++].label = strdup(label);
}

int GraphContext_GetLabelID(const char *label, LabelStoreType t) {
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
  return GRAPH_NO_LABEL;
}

Index* GraphContext_GetIndex(const char *label, const char *property) {
  Index *idx;
  for (int i = 0; i < gc->index_count; i ++) {
    idx = gc->indices[i];
    if (!strcmp(label, idx->label) && !strcmp(property, idx->property)) return idx;
  }
  return NULL;
}

bool GraphContext_HasIndices() {
  return gc->index_count > 0;
}

void GraphContext_AddIndex(Index *idx) {
  gc->index_count++;
  if (!gc->indices) {
    gc->indices = malloc(gc->index_count * sizeof(Index*));
  } else {
    gc->indices = realloc(gc->indices, gc->index_count * sizeof(Index*));
  }
  gc->indices[gc->index_count - 1] = idx;
}

LabelStore* GraphContext_AllStore(LabelStoreType t) {
  if (t == STORE_NODE) return gc->node_allstore;
  return gc->relation_allstore;
}

LabelStore* GraphContext_GetNodeStore(const char *label) {
  // Check cached stores
  int id = GraphContext_GetLabelID(label, STORE_NODE);
  if (id != GRAPH_NO_LABEL) return gc->node_stores[id];

  // If store is not cached, retrieve from keyspace
  LabelStore *store = LabelStore_Get(gc->ctx, STORE_NODE, gc->graph_name, label);
  if (!store) return NULL;

  assert(store->id < gc->node_count);
  gc->node_stores[store->id] = store;

  return store;
}

void GraphContext_Free() {
  free(gc->graph_name);
  free(gc);
}

