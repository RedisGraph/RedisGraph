/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "graphcontext.h"
#include "graphcontext_type.h"
#include "../util/rmalloc.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

/* Deserialize label store */
void* GraphContextType_RdbLoadStore(RedisModuleIO *rdb, int encver) {
  /* Format:
   * id
   * label
   * #properties
   * properties
   */

  LabelStore *s = rm_calloc(1, sizeof(LabelStore));

  s->id = RedisModule_LoadUnsigned(rdb);
  s->label = rm_strdup(RedisModule_LoadStringBuffer(rdb, NULL));
  s->properties = NewTrieMap();

  uint64_t propCount = RedisModule_LoadUnsigned(rdb);
  for(int i = 0; i < propCount; i++) {
    size_t propLen;
    char *prop = RedisModule_LoadStringBuffer(rdb, &propLen);
    TrieMap_Add(s->properties, prop, propLen, NULL, NULL);
  }

  return s;
}

/* Serialize label store */
void GraphContextType_RdbSaveStore(RedisModuleIO *rdb, void *value) {
  /* Format:
   * id
   * label
   * #properties
   * properties
   */

  LabelStore *s = value;

  RedisModule_SaveUnsigned(rdb, s->id);
  RedisModule_SaveStringBuffer(rdb, s->label, strlen(s->label) + 1);
  RedisModule_SaveUnsigned(rdb, s->properties->cardinality);

  if(s->properties->cardinality) {
    char *ptr;
    tm_len_t len;
    void *v;
    TrieMapIterator *it = TrieMap_Iterate(s->properties, "", 0);
    while(TrieMapIterator_Next(it, &ptr, &len, &v)) {
      RedisModule_SaveStringBuffer(rdb, ptr, len);
    }
    TrieMapIterator_Free(it);
  }
}

void GraphContextType_RdbSave(RedisModuleIO *rdb, void *value) {
  /* Format:
   * graph name
   * #label stores
   * #relation stores
   * #indices
   * label allstore
   * label store X #label stores
   * relation allstore
   * relation store X #relation stores
   * (index label, index property) X #indices
   */

  GraphContext *gc = value;
  RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

  RedisModule_SaveUnsigned(rdb, gc->label_count);
  RedisModule_SaveUnsigned(rdb, gc->relation_count);
  RedisModule_SaveUnsigned(rdb, gc->index_count);

  // Serialize label all store
  GraphContextType_RdbSaveStore(rdb, gc->node_allstore);
  // Serialize each label store
  for (int i = 0; i < gc->label_count; i ++) {
    GraphContextType_RdbSaveStore(rdb, gc->node_stores[i]);
  }

  // Serialize relation all store
  GraphContextType_RdbSaveStore(rdb, gc->relation_allstore);
  // Serialize each relation type store
  for (int i = 0; i < gc->relation_count; i ++) {
    GraphContextType_RdbSaveStore(rdb, gc->relation_stores[i]);
  }

  Index *idx;
  for (int i = 0; i < gc->index_count; i ++) {
    idx = gc->indices[i];
    RedisModule_SaveStringBuffer(rdb, idx->label, strlen(idx->label) + 1);
    RedisModule_SaveStringBuffer(rdb, idx->property, strlen(idx->property) + 1);
  }
}

void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
  /* Format:
   * graph name
   * #label stores
   * #relation stores
   * #indices
   * label allstore
   * label store X #label stores
   * relation allstore
   * relation store X #relation stores
   * (index label, index property) X #indices
   */

  if (encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION) {
    return NULL;
  }

  GraphContext *gc = rm_malloc(sizeof(GraphContext));
  gc->g = NULL;
  // Duplicating string so that it can be safely freed if GraphContext
  // is deleted.
  gc->graph_name = rm_strdup(RedisModule_LoadStringBuffer(rdb, NULL));

  gc->label_count = RedisModule_LoadUnsigned(rdb);
  gc->relation_count = RedisModule_LoadUnsigned(rdb);
  gc->index_count = RedisModule_LoadUnsigned(rdb);

  gc->label_cap = gc->label_count;
  gc->relation_cap = gc->relation_count;
  gc->index_cap = gc->index_count;

  // Retrieve all store for labels
  gc->node_allstore = GraphContextType_RdbLoadStore(rdb, encver);

  // Retrieve each label store
  gc->node_stores = rm_malloc(gc->label_count * sizeof(LabelStore*));
  for (int i = 0; i < gc->label_count; i ++) {
    gc->node_stores[i] = GraphContextType_RdbLoadStore(rdb, encver);
  }

  // Retrieve all store for relations
  gc->relation_allstore = GraphContextType_RdbLoadStore(rdb, encver);

  // Retrieve each relation type store
  gc->relation_stores = rm_malloc(gc->relation_count * sizeof(LabelStore*));
  for (int i = 0; i < gc->relation_count; i ++) {
    gc->relation_stores[i] = GraphContextType_RdbLoadStore(rdb, encver);
  }

  gc->indices = rm_malloc(gc->index_count * sizeof(Index*));
  for (int i = 0; i < gc->index_count; i ++) {
    gc->indices[i] = rm_malloc(sizeof(Index));
    const char *label = RedisModule_LoadStringBuffer(rdb, NULL);
    const char *property = RedisModule_LoadStringBuffer(rdb, NULL);
    // TODO unsafe; assumes Graph has been loaded
    GraphContext_AddIndex(gc, label, property);
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

