/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef SERIALIZE_SCHEMA_H
#define SERIALIZE_SCHEMA_H

#include "../../graphcontext.h"
#include "../../../redismodule.h"
#include "../../../schema/schema.h"

void RdbSaveSchema(RedisModuleIO *rdb, Schema *s);

#endif
