/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>
#include "redismodule.h"
#define VKEY_ENTITY_COUNT_UNLIMITED UINT64_MAX

// TODO tmp
typedef enum {
	LARGE_DEFAULT_NO_THRESHOLD = 0,
	SMALL_DEFAULT_NO_THRESHOLD = 1,
	LARGE_DEFAULT_SMALL_THRESHOLD = 2,
	SMALL_DEFAULT_LARGE_THRESHOLD = 3,
	LARGE_DEFAULT_PCT_THRESHOLD = 4,
	SMALL_DEFAULT_PCT_THRESHOLD = 5,
} ThresholdPolicy;

typedef struct {
	int thread_count;                  // Thread count for thread pool.
	bool async_delete;                 // If true, graph deletion is done asynchronously.
	uint64_t cache_size;               // The cache size for each thread, per graph.
	int omp_thread_count;              // Maximum number of OpenMP threads.
	bool node_creation_buffer;         // If true, size matrices to accommodate future node creations.
	uint64_t vkey_entity_count;        // The limit of number of entities encoded at once for each RDB key.
	bool maintain_transposed_matrices; // If true, maintain a transposed version of each relationship matrix.
	ThresholdPolicy threshold_policy; // TODO tmp
	uint64_t graph_size_default;
} RG_Config;

// Set module-level configurations to defaults or to user arguments where provided.
// Returns REDISMODULE_OK on success, emits an error and returns REDISMODULE_ERR on failure.
int Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// Return the module thread pool size.
int Config_GetThreadCount(void);

// Return the max number of OpenMP threads or -1 if using the default value.
int Config_GetOMPThreadCount(void);

// Return the module virtual key entity limit.
uint64_t Config_GetVirtualKeyEntityCount(void);

// Return true if we are maintaining persistent transposed matrices.
bool Config_MaintainTranspose(void);

// Return the cache size.
uint64_t Config_GetCacheSize(void);

// Return true if graph deletion is done asynchronously.
bool Config_GetAsyncDelete(void);

// Return true if keeping overhead to accommodate future node creations.
bool Config_GetNodeCreationBuffer(void);

