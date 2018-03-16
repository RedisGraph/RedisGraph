#ifndef __TRIEMAP_TYPE_H__
#define __TRIEMAP_TYPE_H__

#include "../../redismodule.h"

extern RedisModuleType *TrieRedisModuleType;

#define TRIEMAP_TYPE_ENCODING_VERSION 1

/* Commands related to the redis TrieType registration */
int TrieMapType_Register(RedisModuleCtx *ctx);
void* TrieMapType_RdbLoad(RedisModuleIO *rdb, int encver);
void TrieMapType_RdbSave(RedisModuleIO *rdb, void *value);
void TrieMapType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value);
void TrieMapType_Free(void *value);

#endif