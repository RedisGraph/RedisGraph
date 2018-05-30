#ifndef GRAPH_TYPE_H
#define GRAPH_TYPE_H

#include "../redismodule.h"

extern RedisModuleType *GraphRedisModuleType;

#define GRAPH_TYPE_ENCODING_VERSION 1

/* Commands related to the redis Graph registration */
int GraphType_Register(RedisModuleCtx *ctx);
void* GraphType_RdbLoad(RedisModuleIO *rdb, int encver);
void GraphType_RdbSave(RedisModuleIO *rdb, void *value);
void GraphType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);
void GraphType_Free(void *value);

#endif