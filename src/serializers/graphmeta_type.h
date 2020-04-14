/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "serializers_include.h"

extern RedisModuleType *GraphMetaRedisModuleType;

/* Commands related to the RedisGraph module registration */
int GraphMetaType_Register(RedisModuleCtx *ctx);
