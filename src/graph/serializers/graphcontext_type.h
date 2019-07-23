/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GRAPHCONTEXT_TYPE_H
#define GRAPHCONTEXT_TYPE_H

#include "../../redismodule.h"

extern RedisModuleType *GraphContextRedisModuleType;

#define GRAPHCONTEXT_TYPE_ENCODING_VERSION 4

/* Commands related to the RedisGraph module registration */
int GraphContextType_Register(RedisModuleCtx *ctx);
void *GraphContextType_RdbLoad(RedisModuleIO *rdb, int encver);
void GraphContextType_RdbSave(RedisModuleIO *rdb, void *value);
void GraphContextType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key,
								 void *value);
void GraphContextType_Free(void *value);

#endif
