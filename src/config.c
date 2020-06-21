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

#define CACHE_SIZE "CACHE_SIZE"  // Config param, the size of each thread cache size, per graph.
#define THREAD_COUNT "THREAD_COUNT" // Config param, number of threads in thread pool
#define OMP_THREAD_COUNT "OMP_THREAD_COUNT" // Config param, max number of OpenMP threads
#define VKEY_MAX_ENTITY_COUNT "VKEY_MAX_ENTITY_COUNT" // Config param, max number of entities in each virtual key
#define MAINTAIN_TRANSPOSED_MATRICES "MAINTAIN_TRANSPOSED_MATRICES" // Whether the module should maintain transposed relationship matrices

#define CACHE_SIZE_DEFAULT 25
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
		RedisModule_Log(ctx, "warning", "Received invalid value '%s' as thread count argument",
						invalid_arg);
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
		RedisModule_Log(ctx, "warning", "Specified invalid maximum '%s' for OpenMP thread count",
						invalid_arg);
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
static int _Config_BuildTransposedMatrices(RedisModuleCtx *ctx, RedisModuleString *build_str) {
	const char *should_build = RedisModule_StringPtrLen(build_str, NULL);
	if(!strcasecmp(should_build, "yes")) {
		config.maintain_transposed_matrices = true;
		RedisModule_Log(ctx, "notice", "Maintaining transposed copies of relationship matrices.");
	} else if(!strcasecmp(should_build, "no")) {
		config.maintain_transposed_matrices = false;
		RedisModule_Log(ctx, "notice", "Not maintaining transposed copies of relationship matrices.");
	} else {
		// Exit with error if argument was not "yes" or "no".
		RedisModule_Log(ctx, "warning",
						"Invalid argument '%s' for maintain_transposed_matrices, expected 'yes' or 'no'", should_build);
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}

// If the user has specified the cache size, update the configuration.
// Returns REDISMODULE_OK on success and REDISMODULE_ERR if the argument was invalid.
static int _Config_SetCacheSize(RedisModuleCtx *ctx, RedisModuleString *cache_size_str) {
	long long cache_size;
	int res = _Config_ParsePositiveInteger(cache_size_str, &cache_size);
	// Exit with error if integer parsing fails.
	if(res != REDISMODULE_OK) {
		const char *invalid_arg = RedisModule_StringPtrLen(cache_size_str, NULL);
		RedisModule_Log(ctx, "warning", "Could not parse cache size argument '%s' as an integer",
						invalid_arg);
		return REDISMODULE_ERR;
	}

	// Log the cache size.
	RedisModule_Log(ctx, "notice", "Cache size is set to %lld.",
					cache_size);

	// Update the cache size in the configuration.
	config.cache_size = cache_size;

	return REDISMODULE_OK;
}

// Initialize every module-level configuration to its default value.
static void _Config_SetToDefaults(RedisModuleCtx *ctx) {
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

	// MEMCHECK compile flag;
	#ifdef MEMCHECK
		// Disable async delete during memcheck.
		config.async_delete = false;
		RedisModule_Log(ctx, "notice", "Graph deletion will be done synchronously.");
	#else
		// Always perform async delete when no checking for memory issues.
		config.async_delete = true;
		RedisModule_Log(ctx, "notice", "Graph deletion will be done asynchronously.");
	#endif
	// Always build transposed matrices by default.
	config.maintain_transposed_matrices = true;
	config.cache_size = CACHE_SIZE_DEFAULT;
}

int Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// Initialize the configuration to its default values.
	_Config_SetToDefaults(ctx);

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
		} else if(!strcasecmp(param, MAINTAIN_TRANSPOSED_MATRICES)) {
			// User specified whether or not to maintain transposed matrices.
			res = _Config_BuildTransposedMatrices(ctx, val);
		} else if(!(strcasecmp(param, CACHE_SIZE))) {
			res = _Config_SetCacheSize(ctx, val);
		} else {
			RedisModule_Log(ctx, "warning", "Encountered unknown module argument '%s'", param);
			return REDISMODULE_ERR;
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

inline bool Config_MaintainTranspose() {
	return config.maintain_transposed_matrices;
}

uint64_t Config_GetCacheSize() {
	return config.cache_size;
}

bool Config_GetAsyncDelete(void) {
	return config.async_delete;
}
