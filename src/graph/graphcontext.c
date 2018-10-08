#include "graphcontext.h"
#include "graphcontext_type.h"

_Thread_local GraphContext *gc;

void _contextRebuild(RedisModuleCtx *ctx) {

}

void GraphContext_New(RedisModuleCtx *ctx, const char *graph_name) {
  gc = malloc(sizeof(GraphContext));
  gc->graph_name = strdup(graph_name);
  gc->label_strings = malloc(sizeof(char*) * GRAPH_DEFAULT_LABEL_CAP);

  // TODO I will probably make a macro for building context keys on the stack
  int keylen = strlen(graph_name) + 4;
  char strKey[keylen + 1];
  strKey[0] = '\0';
  strcat(strKey, graph_name);
  strcat(strKey, "_ctx");

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

  RedisModule_CloseKey(key);
}

bool GraphContext_HasIndices() {
  return gc->index_count > 0;
}

// TODO what to return from these 2
const char* GraphContext_GetLabelFromID(int id) {
  // assert(gc && label_id < gc->label_count);
  assert(gc);
  return gc->label_strings[id];
}

int GraphContext_GetLabelIDFromString(const char *label) {
  // assert(gc && label_id < gc->label_count);
  assert(gc);
  for (int i = 0; i < gc->label_count; i ++) {
    if (!strcmp(label, gc->label_strings[i])) return i;
  }
  assert(0);
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

void GraphContext_Free() {
  free(gc->graph_name);
  free(gc);
}

