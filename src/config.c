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
#define PRIORITIZE_MEMORY "PRIORITIZE_MEMORY" // Config param, set all configs that reduce memory overhead
#define NODE_CREATION_BUFFER "NODE_CREATION_BUFFER" // Config param, true if we save space for future nodes
#define VKEY_MAX_ENTITY_COUNT "VKEY_MAX_ENTITY_COUNT" // Config param, max number of entities in each virtual key
#define MAINTAIN_TRANSPOSED_MATRICES "MAINTAIN_TRANSPOSED_MATRICES" // Whether the module should maintain transposed relationship matrices

#define CACHE_SIZE_DEFAULT 25
#define VKEY_MAX_ENTITY_COUNT_DEFAULT 100000

extern RG_Config config; // Global module configuration.

// Parse a setting that should be "yes" or "no".
static inline int _Config_ParseYesNoChoice(RedisModuleString *arg_str, bool *choice) {
	const char *arg = RedisModule_StringPtrLen(arg_str, NULL);
	if(!strcasecmp(arg, "yes")) {
		*choice = true;
	} else if(!strcasecmp(arg, "no")) {
		*choice = false;
	} else {
		// Exit with error if argument was not "yes" or "no".
		RedisModule_Log(NULL, "warning", "Received invalid argument '%s', expected 'yes' or 'no'", arg);
		return REDISMODULE_ERR;
	}
	return REDISMODULE_OK;
}

// Parse a setting that should be a positive 32-bit integer.
static inline int _Config_ParsePositiveInteger(RedisModuleString *integer_str, uint *value) {
	long long longval;
	int res = RedisModule_StringToLongLong(integer_str, &longval);
	// Return an error code if integer parsing fails or value is not positive.
	if(res != REDISMODULE_OK || longval <= 0 || longval > UINT_MAX) {
		RedisModule_Log(NULL, "warning", "Could not parse argument '%s' as non-negative integer",
						integer_str);
		return REDISMODULE_ERR;
	}
	*value = (uint)longval;
	return REDISMODULE_OK;
}

static int _Config_SetPrioritizeMemory(RedisModuleString *arg) {
	bool prioritize_memory;
	if(_Config_ParseYesNoChoice(arg, &prioritize_memory) != REDISMODULE_OK) return REDISMODULE_ERR;
	if(prioritize_memory) {
		config.maintain_transposed_matrices = false;
		config.node_creation_buffer = false;
	} else {
		config.maintain_transposed_matrices = true;
		config.node_creation_buffer = true;
	}
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

	// MEMCHECK compile flag;
	#ifdef MEMCHECK
		// Disable async delete during memcheck.
		config.async_delete = false;
		RedisModule_Log(NULL, "notice", "Graph deletion will be done synchronously.");
	#else
		// Always perform async delete when no checking for memory issues.
		config.async_delete = true;
		RedisModule_Log(NULL, "notice", "Graph deletion will be done asynchronously.");
	#endif
	config.node_creation_buffer = true;
	config.cache_size = CACHE_SIZE_DEFAULT;
	config.maintain_transposed_matrices = true;
}

// Parse configurations that modify multiple settings.
static int _Config_ParseUmbrellaConfigs(RedisModuleString **argv, int argc) {
	for(int cur = 0; cur < argc; cur += 2) {
		// Each configuration is a key-value pair.
		const char *param = RedisModule_StringPtrLen(argv[cur], NULL);
		RedisModuleString *val = argv[cur + 1];

		if(!(strcasecmp(param, PRIORITIZE_MEMORY))) {
			if(_Config_SetPrioritizeMemory(val) != REDISMODULE_OK) return REDISMODULE_ERR;
		}
	}

	return REDISMODULE_OK;
}

// Parse all single configurations.
static int _Config_ParseConfigs(RedisModuleString **argv, int argc) {
	int res = REDISMODULE_OK;
	for(int cur = 0; cur < argc; cur += 2) {
		// Each configuration is a key-value pair.
		const char *param = RedisModule_StringPtrLen(argv[cur], NULL);
		RedisModuleString *val = argv[cur + 1];

		if(!strcasecmp(param, THREAD_COUNT)) {
			// User defined size of thread pool.
			int cpu_count = config.thread_count;
			res = _Config_ParsePositiveInteger(val, &config.thread_count);
			if(config.thread_count > cpu_count) {
				// Emit notice but do not fail if the specified thread count is greater than the system's
				// number of cores (which is already set as the default value).
				RedisModule_Log(NULL, "notice", "Number of threads: %d greater then number of cores: %d.",
								config.thread_count, cpu_count);
			}
		} else if(!strcasecmp(param, OMP_THREAD_COUNT)) {
			// User defined maximum number of OpenMP threads.
			res = _Config_ParsePositiveInteger(val, &config.omp_thread_count);
		} else if(!strcasecmp(param, VKEY_MAX_ENTITY_COUNT)) {
			// User defined maximum number of entities per virtual key.
			res = _Config_ParsePositiveInteger(val, &config.vkey_entity_count);
		} else if(!strcasecmp(param, MAINTAIN_TRANSPOSED_MATRICES)) {
			// User specified whether or not to maintain transposed matrices.
			res = _Config_ParseYesNoChoice(val, &config.maintain_transposed_matrices);
		} else if(!(strcasecmp(param, CACHE_SIZE))) {
			// User defined query cache size.
			res = _Config_ParsePositiveInteger(val, &config.cache_size);
		} else if(!(strcasecmp(param, NODE_CREATION_BUFFER))) {
			// User-defined toggle to maintain node creation buffer.
			res = _Config_ParseYesNoChoice(val, &config.node_creation_buffer);
		} else if(!(strcasecmp(param, PRIORITIZE_MEMORY))) {
			continue; // Already processed umbrella parameters.
		} else {
			RedisModule_Log(NULL, "warning", "Encountered unknown module argument '%s'", param);
			return REDISMODULE_ERR;
		}

		// Exit if we encountered an error.
		if(res != REDISMODULE_OK) return res;
	}

	return REDISMODULE_OK;
}

static void _Config_LogSettings(void) {
	// Log the virtual key entity threshold.
	RedisModule_Log(NULL, "notice", "Max number of entities per graph meta key set to %lld.",
					config.vkey_entity_count);

	if(config.maintain_transposed_matrices) {
		RedisModule_Log(NULL, "notice", "Maintaining transposed copies of relationship matrices.");
	} else {
		RedisModule_Log(NULL, "notice", "Not maintaining transposed copies of relationship matrices.");
	}

	if(config.node_creation_buffer) {
		RedisModule_Log(NULL, "notice", "Maintaining node creation buffer");
	} else {
		RedisModule_Log(NULL, "notice", "Not maintaining node creation buffer");
	}

	// Log the cache size.
	RedisModule_Log(NULL, "notice", "Cache size is set to %lld.", config.cache_size);
}

int Config_Init(RedisModuleString **argv, int argc) {
	// Initialize the configuration to its default values.
	_Config_SetToDefaults();

	if(argc % 2) {
		// Emit an error if we received an odd number of arguments, as this indicates an invalid configuration.
		RedisModule_Log(NULL, "warning",
						"RedisGraph received %d arguments, all configurations should be key-value pairs", argc);
		return REDISMODULE_ERR;
	}


	if(_Config_ParseUmbrellaConfigs(argv, argc) != REDISMODULE_OK) return REDISMODULE_ERR;
	if(_Config_ParseConfigs(argv, argc) != REDISMODULE_OK) return REDISMODULE_ERR;

	_Config_LogSettings();

	return REDISMODULE_OK;
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

bool Config_GetNodeCreationBuffer() {
	return config.node_creation_buffer;
}

