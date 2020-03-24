/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../redismodule.h"


extern RedisModuleType *GraphMetaRedisModuleType;

/* Commands related to the RedisGraph module registration */
int GraphMetaType_Register(RedisModuleCtx *ctx);
void *GraphMetaType_RdbLoad(RedisModuleIO *rdb, int encver);
void GraphMetaType_RdbSave(RedisModuleIO *rdb, void *value);
void GraphMetaType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);
void GraphMetaType_Free(void *value);

