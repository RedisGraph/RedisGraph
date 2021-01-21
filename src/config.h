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

typedef enum {
	Config_CACHE_SIZE               = 0,  // number of entries in cache
	Config_ASYNC_DELETE             = 1,  // delete graph asynchronously
	Config_OPENMP_NTHREAD           = 2,  // max number of OpenMP threads to use
	Config_THREAD_POOL_SIZE         = 3,  // number of threads in thread pool
	Config_RESULTSET_MAX_SIZE       = 4,  // max number of records in result-set
	Config_MAINTAIN_TRANSPOSE       = 5,  // maintain transpose matrices
	Config_VKEY_MAX_ENTITY_COUNT    = 6,  // max number of elements in vkey
	Config_MEMORY_PROTECTION        = 8,  // protect from exploding the memory
	Config_END_MARKER               = 9
} Config_Option_Field;

// configuration object
typedef struct {
	bool async_delete;                 // If true, graph deletion is done asynchronously.
	uint64_t cache_size;               // The cache size for each thread, per graph.
	uint thread_pool_size;             // Thread count for thread pool.
	uint omp_thread_count;             // Maximum number of OpenMP threads.
	uint64_t resultset_size;           // resultset maximum size, (-1) unlimited
	uint64_t vkey_entity_count;        // The limit of number of entities encoded at once for each RDB key.
	bool maintain_transposed_matrices; // If true, maintain a transposed version of each relationship matrix.
	bool memory_protection;
} RG_Config;

// Run-time configurable fields
#define RUNTIME_CONFIG_COUNT 1
static const Config_Option_Field RUNTIME_CONFIGS[] = { Config_RESULTSET_MAX_SIZE };

// Set module-level configurations to defaults or to user arguments where provided.
// returns REDISMODULE_OK on success, emits an error and returns REDISMODULE_ERR on failure.
int Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// returns true if 'field_str' reffers to a configuration field and sets
// 'field' accordingly
bool Config_Contains_field(const char *field_str, Config_Option_Field *field);

// returns field name
const char *Config_Field_name(Config_Option_Field field);

bool Config_Option_set(Config_Option_Field field, RedisModuleString *val);

bool Config_Option_get(Config_Option_Field field, ...);

