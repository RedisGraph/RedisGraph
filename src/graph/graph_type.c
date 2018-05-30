#include "graph.h"
#include "graph_type.h"

/* Declaration of the type for redis registration. */
RedisModuleType *GraphRedisModuleType;

void _GraphType_LoadNodes(RedisModuleIO *rdb, Graph *g, size_t node_count) {
    // TODO: implement.
}

void _GraphType_LoadMatrices(RedisModuleIO *rdb, Graph *g) {
    // TODO: implement.
}

void _GraphType_SaveNodes(RedisModuleIO *rdb, Graph *g) {
    // TODO: implement.
}
void _GraphType_SaveMatrices(RedisModuleIO *rdb, Graph *g) {
    // TODO: implement.
}

void *GraphType_RdbLoad(RedisModuleIO *rdb, int encver) {
    if (encver != 0) {
        return NULL;
    }

    // Determin how many nodes are in the graph.
    uint64_t node_count = RedisModule_LoadUnsigned(rdb);
    Graph *g = Graph_New(node_count);

    // Load nodes.
    _GraphType_LoadNodes(rdb, g, node_count);

    // Determin how matrices are in the graph.
    _GraphType_LoadMatrices(rdb, g);

    return g;
}

void GraphType_RdbSave(RedisModuleIO *rdb, void *value) {
    Graph *g = (Graph *)value;
    
    // Dump number of nodes.
    RedisModule_SaveUnsigned(rdb, g->node_count);
    
    // Dump nodes.
    _GraphType_SaveNodes(rdb, g);

    // Dump number of matrices.
    RedisModule_SaveUnsigned(rdb, g->relation_count);
    
    // Dump matrices.
    _GraphType_SaveMatrices(rdb, g);
}

void GraphType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
  // TODO: implement.
}

void GraphType_Free(void *value) {
  Graph *g = value;
  Graph_Free(g);
}

int GraphType_Register(RedisModuleCtx *ctx) {
  RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                               .rdb_load = GraphType_RdbLoad,
                               .rdb_save = GraphType_RdbSave,
                               .aof_rewrite = GraphType_AofRewrite,
                               .free = GraphType_Free};
  
  
  GraphRedisModuleType = RedisModule_CreateDataType(ctx, "graphtype", GRAPH_TYPE_ENCODING_VERSION, &tm);
  if (GraphRedisModuleType == NULL) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}
