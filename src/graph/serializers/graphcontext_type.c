/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../graphcontext.h"
#include "graphcontext_type.h"
#include "serialize_graph.h"
#include "serialize_schema.h"
#include "serialize_index.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"
#include "../../version.h"

/* Thread local storage graph context key. */
extern pthread_key_t _tlsGCKey;

/* Declaration of the type for redis registration. */
RedisModuleType *GraphContextRedisModuleType;

void static _GraphContextType_SerializeIndicies(RedisModuleIO *rdb, GraphContext *gc) {
  // Currently indicies are only defined on nodes.
  unsigned short schema_count = GraphContext_SchemaCount(gc, SCHEMA_NODE);

  for (unsigned short i = 0; i < schema_count; i ++) {
    Schema *s = gc->node_schemas[i];
    unsigned short index_count = Schema_IndexCount(s);

    for(unsigned short j = 0; j < index_count; j++) {
      Index *idx = s->indices[j];
      RdbSaveIndex(rdb, idx);
    }
  }
}

void GraphContextType_RdbSave(RedisModuleIO *rdb, void *value) {
  /* Format:
   * graph name
   * #node schemas
   * unified node schema
   * node schema X #node schemas
   * #relation schemas
   * unified relation schema
   * relation schema X #relation schemas
   * graph object
   * #indices
   * (index label, index property) X #indices
   */

  GraphContext *gc = value;

  // Lock.
  Graph_AcquireReadLock(gc->g);

  // Graph name.
  RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

  // #Node schemas.
  unsigned short schema_count = GraphContext_SchemaCount(gc, SCHEMA_NODE);
  RedisModule_SaveUnsigned(rdb, schema_count);

  // Serialize all attribute keys
  RdbSaveAttributeKeys(rdb, gc);

  // Name of label X #node schemas.
  for(int i = 0; i < schema_count; i++) {
    Schema *s = gc->node_schemas[i];
    RdbSaveSchema(rdb, s);
  }

  // #Relation schemas.
  unsigned short relation_count = GraphContext_SchemaCount(gc, SCHEMA_EDGE);
  RedisModule_SaveUnsigned(rdb, relation_count);

  // Serialize unified edge schema.
  RdbSaveDummySchema(rdb);

  // Name of label X #relation schemas.
  for(unsigned short i = 0; i < relation_count; i++) {
    Schema *s = gc->relation_schemas[i];
    RdbSaveSchema(rdb, s);
  }

  // Serialize graph object
  RdbSaveGraph(rdb, gc->g, gc->string_mapping);

  // #Indices.
  uint32_t index_count = gc->index_count;
  RedisModule_SaveUnsigned(rdb, index_count);

  // Serialize each index
  _GraphContextType_SerializeIndicies(rdb, gc);

  // Unlock.
  Graph_ReleaseLock(gc->g);
}

void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver) {
  /* Format:
   * graph name
   * #node schemas
   * unified node schema
   * node schema X #node schemas
   * #relation schemas
   * unified relation schema
   * relation schema X #relation schemas
   * graph object
   * #indices
   * (index label, index property) X #indices
   */

  if (encver > GRAPHCONTEXT_TYPE_ENCODING_VERSION) {
    // Not forward compatible.
    printf("Failed loading Graph, RedisGraph version (%d) is not forward compatible.\n", REDISGRAPH_MODULE_VERSION);
    return NULL;
  }

  // TODO can have different functions for different versions here if desired

  GraphContext *gc = rm_malloc(sizeof(GraphContext));
  
  // No indicies.
  gc->index_count = 0;
  
  // _tlsGCKey was created as part of module load.
  pthread_setspecific(_tlsGCKey, gc);

  // Graph name
  gc->graph_name = RedisModule_LoadStringBuffer(rdb, NULL);

  gc->g = Graph_New(GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);

  // #Node schemas
  uint32_t schema_count = RedisModule_LoadUnsigned(rdb);

  // Initialize property mappings
  gc->attributes = NewTrieMap();
  gc->string_mapping = array_new(char*, 64);

  // unified node schema.
  RdbLoadAttributeKeys(rdb, gc);

  // Load each node schema
  gc->node_schemas = array_new(Schema*, schema_count);
  for (uint32_t i = 0; i < schema_count; i ++) {
    array_append(gc->node_schemas, RdbLoadSchema(rdb, SCHEMA_NODE));
    Graph_AddLabel(gc->g);
  }

  // #Edge schemas
  schema_count = RedisModule_LoadUnsigned(rdb);

  // unified edge schema.
  RdbLoadAttributeKeys(rdb, gc);

  // Load each edge schema
  gc->relation_schemas = array_new(Schema*, schema_count);
  for (uint32_t i = 0; i < schema_count; i ++) {
    array_append(gc->relation_schemas, RdbLoadSchema(rdb, SCHEMA_EDGE));
    Graph_AddRelationType(gc->g);
  }

  // Graph object.
  RdbLoadGraph(rdb, gc->g, gc->string_mapping);

  // #Indices
  // (index label, index property) X #indices
  uint32_t index_count = RedisModule_LoadUnsigned(rdb);
  for (uint32_t i = 0; i < index_count; i ++) {
    RdbLoadIndex(rdb, gc);
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

