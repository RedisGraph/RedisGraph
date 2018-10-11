/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "graphcontext.h"
#include "graphcontext_type.h"
#include "../index/index_type.h"
#include "../stores/store_type.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

void GraphContextType_RdbSave(RedisModuleIO *rdb, void *value) {
  GraphContext *gc = value;
  RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

  RedisModule_SaveUnsigned(rdb, gc->label_cap);
  RedisModule_SaveUnsigned(rdb, gc->relation_cap);
  RedisModule_SaveUnsigned(rdb, gc->relation_count);
  RedisModule_SaveUnsigned(rdb, gc->label_count);
  RedisModule_SaveUnsigned(rdb, gc->index_count);

  // Serialize label all store
  StoreType_RdbSave(rdb, gc->node_allstore);
  // Serialize each label store
  for (int i = 0; i < gc->label_count; i ++) {
    StoreType_RdbSave(rdb, gc->node_stores[i]);
  }

  // Serialize relation all store
  StoreType_RdbSave(rdb, gc->relation_allstore);
  // Serialize each relation type store
  for (int i = 0; i < gc->relation_count; i ++) {
    StoreType_RdbSave(rdb, gc->relation_stores[i]);
  }

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
  gc->g = NULL;
  gc->graph_name = RedisModule_LoadStringBuffer(rdb, NULL);

  gc->label_cap = RedisModule_LoadUnsigned(rdb);
  gc->relation_cap = RedisModule_LoadUnsigned(rdb);
  gc->relation_count = RedisModule_LoadUnsigned(rdb);
  gc->label_count = RedisModule_LoadUnsigned(rdb);
  gc->index_count = RedisModule_LoadUnsigned(rdb);

  // Retrieve all store for labels
  gc->node_allstore = StoreType_RdbLoad(rdb, encver);

  // Retrieve each label store
  gc->node_stores = malloc(gc->label_cap * sizeof(LabelStore*));
  for (int i = 0; i < gc->label_count; i ++) {
    gc->node_stores[i] = StoreType_RdbLoad(rdb, encver);
  }

  // Retrieve all store for relations
  gc->relation_allstore = StoreType_RdbLoad(rdb, encver);

  // Retrieve each relation type store
  gc->relation_stores = malloc(gc->relation_cap * sizeof(LabelStore*));
  for (int i = 0; i < gc->relation_count; i ++) {
    gc->relation_stores[i] = StoreType_RdbLoad(rdb, encver);
  }

  gc->indices = NULL;
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

