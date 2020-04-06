#include "../../../../graphcontext.h"
#include "../../../../../index/index.h"
#include "../../../../../redismodule.h"
#include "../../../../../schema/schema.h"

GraphContext *RdbLoadGraphContext_v7(RedisModuleIO *rdb);
void RdbLoadNodes_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadNodes_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadDeletedNodes_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadDeletedEdges_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadEdges_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadGraphSchema_v7(RedisModuleIO *rdb, GraphContext *gc);
