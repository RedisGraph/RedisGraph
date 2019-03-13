/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef SERIALIZE_SCHEMA_H
#define SERIALIZE_SCHEMA_H

#include "../../redismodule.h"
#include "../../schema/schema.h"

Schema* RdbLoadUnifiedSchema(RedisModuleIO *rdb, TrieMap *attributes, char **string_mapping);
Schema* RdbLoadSchema(RedisModuleIO *rdb, SchemaType type);
void RdbSaveSchema(RedisModuleIO *rdb, void *value);
void RdbSaveUnifiedSchema(RedisModuleIO *rdb, void *value, const char **string_mapping);

#endif
