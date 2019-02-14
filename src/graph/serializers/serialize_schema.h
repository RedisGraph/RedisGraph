/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */


#ifndef SERIALIZE_SCHEMA_H
#define SERIALIZE_SCHEMA_H

#include "../../redismodule.h"
#include "../../schema/schema.h"

Schema* RdbLoadUnifiedSchema(RedisModuleIO *rdb);
Schema* RdbLoadSchema(RedisModuleIO *rdb, SchemaType type);
void RdbSaveSchema(RedisModuleIO *rdb, void *value);

#endif
