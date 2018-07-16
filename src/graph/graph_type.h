/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

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