/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>
#include "redismodule.h"

typedef struct {
	int thread_count;            // Thread count for thread pool.                 // Thread count for thread pool.
	bool async_delete;           // If true, graph deletion is done asynchronously.
	int omp_thread_count;        // Maximum number of OpenMP threads.
} RG_Config;

// Set module-level configurations to defaults or to user arguments where provided.
// Returns REDISMODULE_OK on success, emits an error and returns REDISMODULE_ERR on failure.
int Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// Return the module thread pool size.
int Config_GetThreadCount(void);

// Return the max number of OpenMP threads or -1 if using the default value.
int Config_GetOMPThreadCount(void);

// Return true if graph deletion is done asynchronously.
bool Config_GetAsyncDelete(void);
