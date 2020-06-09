/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "config.h"
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "util/redis_version.h"
#include "../deps/GraphBLAS/Include/GraphBLAS.h"

#define THREAD_COUNT "THREAD_COUNT" // Config param, number of threads in thread pool
#define OMP_THREAD_COUNT "OMP_THREAD_COUNT" // Config param, max number of OpenMP threads
#define VKEY_MAX_ENTITY_COUNT "VKEY_MAX_ENTITY_COUNT" // Config param, max number of entities in each virtual key
#define VKEY_MAX_ENTITY_COUNT_DEFAULT 100000

extern RG_Config config; // Global module configuration.

static inline int _Config_ParsePositiveInteger(RedisModuleString *integer_str, long long *value) {
	int res = RedisModule_StringToLongLong(integer_str, value);
	// Return an error code if integer parsing fails or value is not positive.
	if(res != REDISMODULE_OK || *value <= 0) return REDISMODULE_ERR;
	return REDISMODULE_OK;
}

// If the user has specified a thread pool size, update the configuration.
// Returns REDISMODULE_OK on success and REDISMODULE_ERR if the argument was invalid.
static int _Config_SetThreadCount(RedisModuleCtx *ctx, RedisModuleString *count_str) {
	long long thread_count;
	int res = _Config_ParsePositiveInteger(count_str, &thread_count);
	// Exit with error if integer parsing fails or thread count is outside of the valid range 1-INT_MAX.
	if(res != REDISMODULE_OK || thread_count > INT_MAX) {
		const char *invalid_arg = RedisModule_StringPtrLen(count_str, NULL);
		RedisModule_Log(ctx, "warning", "Received invalid value %lld as thread count argument",
						thread_count);
		return REDISMODULE_ERR;
	}

	// Emit notice but do not fail if the specified thread count is greater than the system's
	// number of cores (which is already set as the default value).
	if(thread_count > config.thread_count) {
		RedisModule_Log(ctx, "notice", "Number of threads: %d greater then number of cores: %d.",
						thread_count, config.thread_count);
	}

	// Set the thread count in the configuration.
	config.thread_count = thread_count;

	return REDISMODULE_OK;
}

// If the user has specified a maximum number of OpenMP threads, update the configuration.
// Returns REDISMODULE_OK on success and REDISMODULE_ERR if the argument was invalid.
static int _Config_SetOMPThreadCount(RedisModuleCtx *ctx, RedisModuleString *count_str) {
	long long omp_thread_count;
	int res = _Config_ParsePositiveInteger(count_str, &omp_thread_count);
	// Exit with error if integer parsing fails or OpenMP thread count is outside of the valid range 1-INT_MAX.
	if(res != REDISMODULE_OK || omp_thread_count > INT_MAX) {
		const char *invalid_arg = RedisModule_StringPtrLen(count_str, NULL);
		RedisModule_Log(ctx, "warning", "Specified invalid maximum %lld for OpenMP thread count",
						omp_thread_count);
		return REDISMODULE_ERR;
	}

	// Set the OpenMP thread count in the configuration.
	config.omp_thread_count = omp_thread_count;

	return REDISMODULE_OK;
}

// If the user has specified a maximum number of entities per virtual key, update the configuration.
// Returns REDISMODULE_OK on success and REDISMODULE_ERR if the argument was invalid.
static int _Config_SetVirtualKeyEntitiesThreshold(RedisModuleCtx *ctx,
												  RedisModuleString *entity_count_str) {
	long long entity_count;
	int res = _Config_ParsePositiveInteger(entity_count_str, &entity_count);
	// Exit with error if integer parsing fails.
	if(res != REDISMODULE_OK) {
		const char *invalid_arg = RedisModule_StringPtrLen(entity_count_str, NULL);
		RedisModule_Log(ctx, "warning", "Could not parse virtual key size argument '%s' as an integer",
						invalid_arg);
		return REDISMODULE_ERR;
	}

	// Log the new entity threshold.
	RedisModule_Log(ctx, "notice", "Max number of entities per graph meta key set to %lld.",
					entity_count);

	// Update the entity count in the configuration.
	config.vkey_entity_count = entity_count;

	return REDISMODULE_OK;
}

// Initialize every module-level configuration to its default value.
static void _Config_SetToDefaults(void) {
	// The thread pool's default size is equal to the system's number of cores.
	int CPUCount = sysconf(_SC_NPROCESSORS_ONLN);
	config.thread_count = (CPUCount != -1) ? CPUCount : 1;

	// Use the GraphBLAS-defined number of OpenMP threads by default.
	GxB_get(GxB_NTHREADS, &config.omp_thread_count);

	if(Redis_Version_GreaterOrEqual(6, 0, 0)) {
		// The default entity count of virtual keys for server versions >= 6 is set by macro.
		config.vkey_entity_count = VKEY_MAX_ENTITY_COUNT_DEFAULT;
	} else {
		// For redis-server versions below 6.0.0, we will not split the graph into virtual keys.
		config.vkey_entity_count = VKEY_ENTITY_COUNT_UNLIMITED;
	}
}

int Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// Initialize the configuration to its default values.
	_Config_SetToDefaults();

	if(argc % 2) {
		// Emit an error if we received an odd number of arguments, as this indicates an invalid configuration.
		RedisModule_Log(ctx, "warning",
						"RedisGraph received %d arguments, all configurations should be key-value pairs", argc);
		return REDISMODULE_ERR;
	}

	int res = REDISMODULE_OK;
	for(int cur = 0; cur < argc; cur += 2) {
		// Each configuration is a key-value pair.
		const char *param = RedisModule_StringPtrLen(argv[cur], NULL);
		RedisModuleString *val = argv[cur + 1];

		if(!strcasecmp(param, THREAD_COUNT)) {
			// User defined size of thread pool.
			res = _Config_SetThreadCount(ctx, val);
		} else if(!strcasecmp(param, OMP_THREAD_COUNT)) {
			// User defined maximum number of OpenMP threads.
			res = _Config_SetOMPThreadCount(ctx, val);
		} else if(!strcasecmp(param, VKEY_MAX_ENTITY_COUNT)) {
			// User defined maximum number of entities per virtual key.
			res = _Config_SetVirtualKeyEntitiesThreshold(ctx, val);
		} else {
			RedisModule_Log(ctx, "warning", "Encountered unknown module argument '%s'", param);
		}

		// Exit if we encountered an error.
		if(res != REDISMODULE_OK) return res;
	}

	return res;
}

inline int Config_GetThreadCount() {
	return config.thread_count;
}

inline int Config_GetOMPThreadCount() {
	return config.omp_thread_count;
}

inline uint64_t Config_GetVirtualKeyEntityCount() {
	return config.vkey_entity_count;
}

