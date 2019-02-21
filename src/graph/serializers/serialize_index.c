/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "serialize_index.h"

void RdbLoadIndex(RedisModuleIO *rdb, GraphContext *gc) {
    char *label = RedisModule_LoadStringBuffer(rdb, NULL);
    char *property = RedisModule_LoadStringBuffer(rdb, NULL);
    GraphContext_AddIndex(gc, label, property);
    RedisModule_Free(label);
    RedisModule_Free(property);
}

void RdbSaveIndex(RedisModuleIO *rdb, void *value) {
    Index *idx = (Index*)value;
    RedisModule_SaveStringBuffer(rdb, idx->label, strlen(idx->label) + 1);
    RedisModule_SaveStringBuffer(rdb, idx->attribute, strlen(idx->attribute) + 1);
}
