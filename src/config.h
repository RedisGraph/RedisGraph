/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "redismodule.h"

typedef struct {
	long long thread_count;      // Thread count for thread pool.
	uint64_t entities_threshold; // The limit of number of entities encoded at once for each RDB key.
} RG_Config;

void Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// Returns the module configuration single instance.
RG_Config Config_GetModuleConfig();
