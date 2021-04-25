/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdbool.h>
#include "redismodule.h"

#define RESULTSET_SIZE_UNLIMITED    UINT64_MAX
#define QUERY_MEM_CAPACITY_UNLIMITED INT64_MIN
#define CONFIG_TIMEOUT_NO_TIMEOUT   0
#define VKEY_ENTITY_COUNT_UNLIMITED UINT64_MAX

typedef enum {
	Config_TIMEOUT                  = 0,  // timeout value for queries
	Config_CACHE_SIZE               = 1,  // number of entries in cache
	Config_ASYNC_DELETE             = 2,  // delete graph asynchronously
	Config_OPENMP_NTHREAD           = 3,  // max number of OpenMP threads to use
	Config_THREAD_POOL_SIZE         = 4,  // number of threads in thread pool
	Config_RESULTSET_MAX_SIZE       = 5,  // max number of records in result-set
	Config_MAINTAIN_TRANSPOSE       = 6,  // maintain transpose matrices
	Config_VKEY_MAX_ENTITY_COUNT    = 7,  // max number of elements in vkey
	Config_MAX_QUEUED_QUERIES       = 8,  // max number of queued queries
	Config_QUERY_MEM_CAPACITY       = 9,  // Max mem(bytes) that query/thread can utilize at any given time
	Config_END_MARKER               = 10
} Config_Option_Field;

typedef void (*Config_on_change)(Config_Option_Field type); // callback function which being called when config param changed

// Run-time configurable fields
#define RUNTIME_CONFIG_COUNT 4
static const Config_Option_Field RUNTIME_CONFIGS[] =
{
	Config_RESULTSET_MAX_SIZE,
	Config_TIMEOUT,
	Config_MAX_QUEUED_QUERIES,
	Config_QUERY_MEM_CAPACITY
};

// Set module-level configurations to defaults or to user arguments where provided.
// returns REDISMODULE_OK on success, emits an error and returns REDISMODULE_ERR on failure.
int Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

// returns true if 'field_str' reffers to a configuration field and sets
// 'field' accordingly
bool Config_Contains_field(const char *field_str, Config_Option_Field *field);

// returns field name
const char *Config_Field_name(Config_Option_Field field);

bool Config_Option_set(Config_Option_Field field, const char *val);

bool Config_Option_get(Config_Option_Field field, ...);

// Sets the callback function which will be called when a config param changes.
void Config_Subscribe_Changes(Config_on_change cb);

