/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __INDEX_TYPE_H__
#define __INDEX_TYPE_H__

#include "index.h"
#include "../redismodule.h"

extern RedisModuleType *IndexRedisModuleType;

#define INDEX_TYPE_ENCODING_VERSION 1

int IndexType_Register(RedisModuleCtx *ctx);
void* IndexType_RdbLoad(RedisModuleIO *rdb, int encver);
void IndexType_RdbSave(RedisModuleIO *rdb, void *value);
void IndexType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);
void IndexType_Free(void *value);

#endif

