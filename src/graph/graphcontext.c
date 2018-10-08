#include "graphcontext.h"
#include "graphcontext_type.h"

_Thread_local GraphContext *gc;

void _contextRebuild(RedisModuleCtx *ctx) {

}

void GraphContext_New(RedisModuleCtx *ctx, const char *graph_name) {
  gc = malloc(sizeof(GraphContext));
  gc->graph_name = strdup(graph_name);
  gc->node_labels = malloc(sizeof(char*) * GRAPH_DEFAULT_NODE_CAP);
  gc->relation_labels = malloc(sizeof(char*) * GRAPH_DEFAULT_RELATION_CAP);

  gc->relation_cap = GRAPH_DEFAULT_RELATION_CAP;
  gc->relation_count = 0;
  gc->node_cap = GRAPH_DEFAULT_NODE_CAP;
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
  gc->edge_allstore = LabelStore_Get(ctx, STORE_EDGE, graph_name, NULL);

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

  // TODO doesn't really belong here
  gc->node_allstore = LabelStore_Get(ctx, STORE_NODE, graph_name, NULL);
  gc->edge_allstore = LabelStore_Get(ctx, STORE_EDGE, graph_name, NULL);

  // Retrieve all label stores?
  // Can iterate over all label strings, but that sounds awful

  RedisModule_CloseKey(key);
}

// TODO next two functions are nearly identical; maybe consolidate
void GraphContext_AddNode(const char *label) {
  if(gc->node_count == gc->node_cap) {
    gc->node_cap += 4;
    gc->node_labels = realloc(gc->node_labels, gc->node_cap * sizeof(char*));
  }
  gc->node_labels[gc->node_count++] = strdup(label);
}

void GraphContext_AddRelation(const char *label) {
  if(gc->relation_count == gc->relation_cap) {
    gc->relation_cap += 4;
    gc->relation_labels = realloc(gc->relation_labels, gc->relation_cap * sizeof(char*));
  }
  gc->relation_labels[gc->relation_count++] = strdup(label);
}

// TODO what to return from these 2
const char* GraphContext_GetLabel(int id) {
  assert(gc && id < gc->node_count);
  return gc->node_labels[id];
}

int GraphContext_GetLabelID(const char *label, LabelStoreType t) {
  char **labels;
  size_t count;
  if (t == STORE_NODE) {
    labels = gc->node_labels;
    count = gc->node_count;
  } else {
    labels = gc->relation_labels;
    count = gc->relation_count;
  }

  for (int i = 0; i < count; i ++) {
    if (!strcmp(label, labels[i])) return i;
  }
  return -1;
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
  gc->indices = realloc(gc->indices, gc->index_count + 1);
  gc->indices[gc->index_count++] = idx;
}

LabelStore* GraphContext_AllStore(LabelStoreType t) {
  if (t == STORE_NODE) return gc->node_allstore;
  return gc->edge_allstore;
}

LabelStore* GraphContext_GetStoreByString(const char *label, LabelStoreType t) {
  int id = GraphContext_GetLabelID(label, t);
  assert(id >= 0);
  if (t == STORE_NODE) return gc->node_stores[id];
  return gc->edge_stores[id];
}

void GraphContext_Free() {
  free(gc->graph_name);
  free(gc);
}

