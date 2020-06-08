/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "redismodule.h"
#define VKEY_ENTITY_COUNT_UNLIMITED UINT64_MAX

typedef struct {
	int omp_thread_count;        // Maximum number of OpenMP threads, -1 indicates the default value.
	long long thread_count;      // Thread count for thread pool.
	uint64_t vkey_entity_count;  // The limit of number of entities encoded at once for each RDB key.
} RG_Config;

void Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// Return the module thread pool size.
long long Config_GetThreadCount(void);

// Return the max number of OpenMP threads or -1 if the default.
int Config_GetOMPThreadCount(void);

// Return the module virtual key entity limit.
uint64_t Config_GetVirtualKeyEntityCount(void);

