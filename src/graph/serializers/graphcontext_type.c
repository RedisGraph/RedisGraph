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
#include "../../util/rmalloc.h"
#include "../../version.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

void GraphContextType_RdbSave(RedisModuleIO *rdb, void *value) {
  /* Format:
   * version
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

  // Version 
  RedisModule_SaveUnsigned(rdb, REDISGRAPH_VERSION_MAJOR);
  RedisModule_SaveUnsigned(rdb, REDISGRAPH_VERSION_MINOR);
  RedisModule_SaveUnsigned(rdb, REDISGRAPH_VERSION_PATCH);
  
  // Graph name.
  GraphContext *gc = value;
  RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

  // #Label stores.
  RedisModule_SaveUnsigned(rdb, gc->label_count + 1);

  // Serialize label all store.
  RdbSaveStore(rdb, gc->node_allstore);
  
  // Name of label X #label stores.
  for(int i = 0; i < gc->label_count; i++) {
    LabelStore *s = gc->node_stores[i];
    RdbSaveStore(rdb, s);
  }

  // #Relation stores.
  RedisModule_SaveUnsigned(rdb, gc->relation_count + 1);
  
  // Serialize relation all store.
  RdbSaveStore(rdb, gc->relation_allstore);

  // Name of relation X #relation.
  for(int i = 0; i < gc->relation_count; i++) {
    LabelStore *s = gc->relation_stores[i];
    RdbSaveStore(rdb, s);
  }

  // Serialize graph object
  RdbSaveGraph(rdb, gc->g);

  // #Indices.
  RedisModule_SaveUnsigned(rdb, gc->index_count);

  // Serialize each index
  Index *idx;
  for (int i = 0; i < gc->index_count; i ++) {
    idx = gc->indices[i];
    RedisModule_SaveStringBuffer(rdb, idx->label, strlen(idx->label) + 1);
    RedisModule_SaveStringBuffer(rdb, idx->property, strlen(idx->property) + 1);
  }
}

void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
  /* Format:
   * version
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
    return NULL;
  }

  // Version 
  uint64_t versionMajor = RedisModule_LoadUnsigned(rdb);
  uint64_t versionMinor = RedisModule_LoadUnsigned(rdb);
  uint64_t versionPatch = RedisModule_LoadUnsigned(rdb);
  int versionSemantic = REDISGRAPH_SEMANTIC_VERSION(versionMajor, versionMinor, versionPatch);
  if(versionSemantic > REDISGRAPH_MODULE_VERSION) {
    // Not forward compatible.
    printf("RedisGraph is not forward compatible, trying to load graph data from version %d using version %d\n",
           versionSemantic, REDISGRAPH_MODULE_VERSION);
    return NULL;
  }

  GraphContext *gc = rm_malloc(sizeof(GraphContext));
  
  // Initialize the generic label and relation stores
  gc->node_stores = NULL;
  gc->relation_stores = NULL;
  gc->indices = NULL;    
  
  // Graph name
  // Duplicating string so that it can be safely freed if GraphContext
  // is deleted.
  gc->graph_name = rm_strdup(RedisModule_LoadStringBuffer(rdb, NULL));
  gc->g = Graph_New(GRAPH_DEFAULT_NODE_CAP);
  
  // #Label stores
  gc->label_count = RedisModule_LoadUnsigned(rdb) - 1;
  gc->label_cap = gc->label_count;

  // label all store.
  gc->node_allstore = RdbLoadStore(rdb);

  // Retrieve each label store
  gc->node_stores = rm_malloc((gc->label_count) * sizeof(LabelStore*));
  for (int i = 0; i < gc->label_count; i ++) {
    gc->node_stores[i] = RdbLoadStore(rdb);
    Graph_AddLabel(gc->g);
  }

  // #Relation stores
  gc->relation_count = RedisModule_LoadUnsigned(rdb) - 1;
  gc->relation_cap = gc->relation_count;

  // Relation all store.
  gc->relation_allstore = RdbLoadStore(rdb);

  // Retrieve each relation store
  gc->relation_stores = rm_malloc((gc->relation_count) * sizeof(LabelStore*));
  for (int i = 0; i < gc->relation_count; i ++) {
    gc->relation_stores[i] = RdbLoadStore(rdb);
    Graph_AddRelationType(gc->g);
  }

  // Graph object.
  RdbLoadGraph(rdb, gc->g);

  // #Indices
  // (index label, index property) X #indices
  uint64_t index_count = RedisModule_LoadUnsigned(rdb);
  gc->index_count = 0;
  gc->index_cap = index_count;
  gc->indices = rm_malloc(gc->index_cap * sizeof(Index*));
  for (int i = 0; i < index_count; i ++) {
    const char *label = RedisModule_LoadStringBuffer(rdb, NULL);
    const char *property = RedisModule_LoadStringBuffer(rdb, NULL);
    GraphContext_AddIndex(gc, label, property);
  }

  // TODO: Try to remove Graph's Graph_AcquireWriteLock(g); from Graph_new.
  GraphContext_Release(gc);
  return gc;
}

void GraphContextType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void GraphContextType_Free(void *value) {
  GraphContext *gc = value;
  Graph_SetSynchronization(gc->g, false);
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
