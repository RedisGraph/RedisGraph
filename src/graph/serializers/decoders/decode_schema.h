/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../graphcontext.h"
#include "../../../redismodule.h"
#include "../../../schema/schema.h"

Schema* RdbLoadSchema(RedisModuleIO *rdb, SchemaType type);
