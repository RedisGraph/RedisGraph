#ifndef STORE_TYPE_H
#define STORE_TYPE_H

#include "../redismodule.h"

extern RedisModuleType *StoreRedisModuleType;

#define STORE_TYPE_ENCODING_VERSION 1

/* Commands related to the redis Store registration */
int StoreType_Register(RedisModuleCtx *ctx);
void* StoreType_RdbLoad(RedisModuleIO *rdb, int encver);
void StoreType_RdbSave(RedisModuleIO *rdb, void *value);
void StoreType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);
void StoreType_Free(void *value);

#endif