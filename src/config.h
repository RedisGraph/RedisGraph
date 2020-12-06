/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>
#include "redismodule.h"

#define RESULTSET_SIZE_UNLIMITED UINT64_MAX
#define VKEY_ENTITY_COUNT_UNLIMITED UINT64_MAX

typedef struct {
	int thread_count;                  // Thread count for thread pool.
	bool async_delete;                 // If true, graph deletion is done asynchronously.
	uint64_t cache_size;               // The cache size for each thread, per graph.
	int omp_thread_count;              // Maximum number of OpenMP threads.
	uint64_t resultset_size;           // resultset maximum size, (-1) unlimited
	uint64_t vkey_entity_count;        // The limit of number of entities encoded at once for each RDB key.
	bool maintain_transposed_matrices; // If true, maintain a transposed version of each relationship matrix.
} RG_Config;

// Set module-level configurations to defaults or to user arguments where provided.
// returns REDISMODULE_OK on success, emits an error and returns REDISMODULE_ERR on failure.
int Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// return the module thread pool size.
int Config_GetThreadCount(void);

// return the max number of OpenMP threads or -1 if using the default value.
int Config_GetOMPThreadCount(void);

// return the module virtual key entity limit.
uint64_t Config_GetVirtualKeyEntityCount(void);

// return true if we are maintaining persistent transposed matrices.
bool Config_MaintainTranspose(void);

// return the cache size.
uint64_t Config_GetCacheSize(void);

// return true if graph deletion is done asynchronously.
bool Config_GetAsyncDelete(void);

// returns resultset size limit
uint64_t Config_GetResultsetSize(void);

