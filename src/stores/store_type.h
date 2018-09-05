/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef STORE_TYPE_H
#define STORE_TYPE_H

#include "../redismodule.h"
#include <stdint.h>

extern RedisModuleType *StoreRedisModuleType;

#define STORE_TYPE_ENCODING_VERSION 1
#define NO_INDEX SIZE_MAX

/* Commands related to the redis Store registration */
int StoreType_Register(RedisModuleCtx *ctx);
void* StoreType_RdbLoad(RedisModuleIO *rdb, int encver);
void StoreType_RdbSave(RedisModuleIO *rdb, void *value);
void StoreType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);
void StoreType_Free(void *value);

#endif
