/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#include "../graphcontext.h"
#include "graphcontext_type.h"
#include "serialize_graph.h"
#include "serialize_store.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../version.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

void GraphContextType_RdbSave(RedisModuleIO *rdb, void *value) {
  /* Format:
   * graph name
   * #label stores
   * label allstore
   * label store X #label stores
   * #relation stores
   * relation allstore
   * relation store X #relation
   * graph object
   * #indices
   * (index label, index property) X #indices
   */

  // Graph name.
  GraphContext *gc = value;
  RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

  // #Label stores.
  uint32_t label_count = array_len(gc->node_stores);
  RedisModule_SaveUnsigned(rdb, label_count);

  // Serialize label all store.
  RdbSaveStore(rdb, gc->node_allstore);

  // Name of label X #label stores.
  for(int i = 0; i < label_count; i++) {
    LabelStore *s = gc->node_stores[i];
    RdbSaveStore(rdb, s);
  }

  // #Relation stores.
  uint32_t relation_count = array_len(gc->relation_stores);
  RedisModule_SaveUnsigned(rdb, relation_count);

  // Serialize relation all store.
  RdbSaveStore(rdb, gc->relation_allstore);

  // Name of relation X #relation.
  for(uint32_t i = 0; i < relation_count; i++) {
    LabelStore *s = gc->relation_stores[i];
    RdbSaveStore(rdb, s);
  }

  // Serialize graph object
  RdbSaveGraph(rdb, gc->g);

  // #Indices.
  uint32_t index_count = array_len(gc->indices);
  RedisModule_SaveUnsigned(rdb, index_count);

  // Serialize each index
  Index *idx;
  for (uint32_t i = 0; i < index_count; i ++) {
    idx = gc->indices[i];
    RedisModule_SaveStringBuffer(rdb, idx->label, strlen(idx->label) + 1);
    RedisModule_SaveStringBuffer(rdb, idx->property, strlen(idx->property) + 1);
  }
}

void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
  /* Format:
   * graph name
   * #label stores
   * label allstore
   * name of label X #label stores
   * #relation stores
   * relation allstore
   * name of relation X #relation
   * graph object
   * #indices
   * (index label, index property) X #indices
   */

  if (encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION) {
    // Not forward compatible.
    printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n", REDISGRAPH_MODULE_VERSION);
    return NULL;
  }

  GraphContext *gc = rm_malloc(sizeof(GraphContext));

  // Graph name
  // Duplicating string so that it can be safely freed if GraphContext
  // is deleted.
  char *graph_name = RedisModule_LoadStringBuffer(rdb, NULL);
  gc->graph_name = rm_strdup(graph_name);
  RedisModule_Free(graph_name);

  gc->g = Graph_New(GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);

  // #Label stores
  uint32_t label_count = RedisModule_LoadUnsigned(rdb);

  // label all store.
  gc->node_allstore = RdbLoadStore(rdb);

  // Retrieve each label store
  gc->node_stores = array_new(LabelStore*, label_count);
  for (uint32_t i = 0; i < label_count; i ++) {
    array_append(gc->node_stores, RdbLoadStore(rdb));
    Graph_AddLabel(gc->g);
  }

  // #Relation stores
  uint32_t relation_count = RedisModule_LoadUnsigned(rdb);

  // Relation all store.
  gc->relation_allstore = RdbLoadStore(rdb);

  // Retrieve each relation store
  gc->relation_stores = array_new(LabelStore*, relation_count);

  for (uint32_t i = 0; i < relation_count; i ++) {
    array_append(gc->relation_stores, RdbLoadStore(rdb));
    Graph_AddRelationType(gc->g);
  }

  // Graph object.
  RdbLoadGraph(rdb, gc->g);

  // #Indices
  // (index label, index property) X #indices
  uint32_t index_count = RedisModule_LoadUnsigned(rdb);
  gc->indices = array_new(Index*, index_count);
  for (uint32_t i = 0; i < index_count; i ++) {
    char *label = RedisModule_LoadStringBuffer(rdb, NULL);
    char *property = RedisModule_LoadStringBuffer(rdb, NULL);
    GraphContext_AddIndex(gc, label, property);
    RedisModule_Free(label);
    RedisModule_Free(property);
  }

  return gc;
}

void GraphContextType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void GraphContextType_Free(void *value) {
  GraphContext *gc = value;
  Graph_SetMatrixPolicy(gc->g, DISABLED);
  GraphContext_Free(gc);
}

int GraphContextType_Register(RedisModuleCtx *ctx) {
  RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                               .rdb_load = GraphContextType_RdbLoad,
                               .rdb_save = GraphContextType_RdbSave,
                               .aof_rewrite = GraphContextType_AofRewrite,
                               .free = GraphContextType_Free};

  GraphContextRedisModuleType = RedisModule_CreateDataType(ctx, "graphdata", GRAPHCONTEXT_TYPE_ENCODING_VERSION, &tm);
  if (GraphContextRedisModuleType == NULL) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}

