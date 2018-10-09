/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "graphcontext.h"
#include "graphcontext_type.h"
#include "../index/index_type.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

void GraphContextType_RdbSave(RedisModuleIO *rdb, void *value) {
  GraphContext *gc = value;
  RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

  RedisModule_SaveUnsigned(rdb, gc->relation_count);
  RedisModule_SaveUnsigned(rdb, gc->node_count);
  RedisModule_SaveUnsigned(rdb, gc->index_count);

  for (int i = 0; i < gc->index_count; i ++) {
    // TODO make better solution
    IndexType_RdbSave(rdb, gc->indices[i]);
  }
}

void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
  if (encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION) {
    return NULL;
  }

  GraphContext *gc = malloc(sizeof(GraphContext));
  gc->graph_name = RedisModule_LoadStringBuffer(rdb, NULL);

  gc->relation_count = RedisModule_LoadUnsigned(rdb);
  gc->node_count = RedisModule_LoadUnsigned(rdb);
  gc->index_count = RedisModule_LoadUnsigned(rdb);

  gc->node_stores = calloc(gc->node_count, sizeof(LabelStore*));
  gc->relation_stores = calloc(gc->relation_count, sizeof(LabelStore*));

  gc->indices = NULL;
  /* TODO I'm curious about the idea of serializing index keys in Redis as before, and
   * retrieving the pointers from the keyspace to save here. That way the keys will still
   * be modular and we won't have a hugely bloated GraphContext value, but once a GraphContext
   * is loaded we won't need to access the keyspace to retrieve indices. */
  if (gc->index_count > 0) {
    gc->indices = malloc(gc->index_count * sizeof(Index*));
    for (int i = 0; i < gc->index_count; i ++) {
      gc->indices[i] = IndexType_RdbLoad(rdb, encver);
    }
  }

  return gc;
}

void GraphContextType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void GraphContextType_Free(void *value) {
  GraphContext *gc = value;
  GraphContext_Free(gc);
}

int GraphContextType_Register(RedisModuleCtx *ctx) {
  RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                               .rdb_load = GraphContextType_RdbLoad,
                               .rdb_save = GraphContextType_RdbSave,
                               .aof_rewrite = GraphContextType_AofRewrite,
                               .free = GraphContextType_Free};

  GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "g_context", GRAPHCONTEXT_TYPE_ENCODING_VERSION, &tm);
  if (GraphContextRedisModuleType == NULL) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}

