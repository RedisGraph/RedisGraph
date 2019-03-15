/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef SERIALIZE_SCHEMA_H
#define SERIALIZE_SCHEMA_H

#include "../../redismodule.h"
#include "../../schema/schema.h"
#include "../graphcontext.h"

void RdbLoadAttributeKeys(RedisModuleIO *rdb, GraphContext *gc);
Schema* RdbLoadSchema(RedisModuleIO *rdb, SchemaType type);

void RdbSaveSchema(RedisModuleIO *rdb, Schema *s);
void RdbSaveDummySchema(RedisModuleIO *rdb);
void RdbSaveAttributeKeys(RedisModuleIO *rdb, GraphContext *gc);

#endif
