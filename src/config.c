/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "config.h"
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "util/redis_version.h"


#define THREAD_COUNT "THREAD_COUNT" // Config param, number of threads in thread pool
#define OMP_THREAD_COUNT "OMP_THREAD_COUNT" // Config param, max number of OpenMP threads
#define VKEY_MAX_ENTITY_COUNT "VKEY_MAX_ENTITY_COUNT" // Config param, number of entities in virtual key
#define VKEY_MAX_ENTITY_COUNT_DEFAULT 100000

extern RG_Config config; // Global module configuration.
static bool _initialized = false;


// Tries to fetch number of threads from
// command line arguments if specified
// otherwise returns thread count equals to the number
// of cores available
static long long _Config_SetThreadCount(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// Default.
	int CPUCount = sysconf(_SC_NPROCESSORS_ONLN);
	long long threadCount = (CPUCount != -1) ? CPUCount : 1;

	// Number of thread specified in configuration?
	// Expecting configuration to be in the form of key value pairs.
	if(argc % 2 == 0) {
		// Scan arguments for THREAD_COUNT.
		for(int i = 0; i < argc; i += 2) {
			const char *param = RedisModule_StringPtrLen(argv[i], NULL);
			if(strcasecmp(param, THREAD_COUNT) == 0) {
				RedisModule_StringToLongLong(argv[i + 1], &threadCount);
				break;
			}
		}
	}

	// Sanity.
	assert(threadCount > 0);
	if(threadCount > CPUCount)
		RedisModule_Log(ctx,
						"warning",
						"Number of threads: %d greater then number of cores: %d.",
						threadCount,
						CPUCount);

	return threadCount;
}

// Scans the LOADMODULE arguments for a user-defined maximum number of OpenMP threads.
// Returns to -1 and does not override default behavior if no such valid argument was found.
static int _Config_SetOMPThreadCount(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	// Scan module arguments for OMP_THREAD_COUNT and extract the value if found.
	for(int i = 0; i < argc; i += 2) {
		const char *param = RedisModule_StringPtrLen(argv[i], NULL);
		if(strcasecmp(param, OMP_THREAD_COUNT)) continue;

		long long omp_thread_count;
		RedisModule_StringToLongLong(argv[i + 1], &omp_thread_count);
		if(omp_thread_count < 1 || omp_thread_count > INT_MAX) {
			// The acceptable range for max OpenMP threads is 1 to INT_MAX.
			RedisModule_Log(ctx, "warning",
							"Specified invalid maximum %d for OpenMP threads",
							omp_thread_count);
			break;
		}
		// A valid maximum was specified, return it.
		return omp_thread_count;
	}

	// Return -1 in the default case or if an invalid configuration was specified.
	return -1;
}

// Tries to fetch the number of entities to encode as part of virtual key encoding.
// Defaults to VKEY_MAX_ENTITY_COUNT_DEFAULT
static uint64_t _Config_SetVirtualKeyEntitiesThreshold(RedisModuleCtx *ctx,
													   RedisModuleString **argv,
													   int argc) {

	// For redis-server versions below 6.0.0, we will not split the graph to virtual keys.
	if(!Redis_Version_GreaterOrEqual(6, 0, 0)) return VKEY_ENTITY_COUNT_UNLIMITED;
	// Default.
	long long threshold = VKEY_MAX_ENTITY_COUNT_DEFAULT;

	// Entities threshold defined in configuration?
	// Expecting configuration to be in the form of key value pairs.
	if(argc % 2 == 0) {
		// Scan arguments for VKEY_MAX_ENTITY_COUNT.
		for(int i = 0; i < argc; i += 2) {
			const char *param = RedisModule_StringPtrLen(argv[i], NULL);
			if(strcasecmp(param, VKEY_MAX_ENTITY_COUNT) == 0) {
				RedisModule_StringToLongLong(argv[i + 1], &threshold);
				break;
			}
		}
	}

	// Sanity.
	assert(threshold > 0 && "A positive integer is required for the number of entities in virtual key");
	RedisModule_Log(ctx, "notice", "Entities limit per graph meta key set to %d.", threshold);

	return (uint64_t)threshold;
}

void Config_Init(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	config.vkey_entity_count = _Config_SetVirtualKeyEntitiesThreshold(ctx, argv, argc);
	config.thread_count = _Config_SetThreadCount(ctx, argv, argc);
	config.omp_thread_count = _Config_SetOMPThreadCount(ctx, argv, argc);
	_initialized = true;
}

/* Static function for:
 * 1. Validation of the configuration object.
 * 2. Future proofing retrieval calls to the configuration object from within config.c.
 * 3. Future proofing retrieval calls to the configuration object outside config.c.
 */
static inline RG_Config _Config_GetModuleConfig() {
	assert(_initialized && "Module configuration was not initialized");
	return config;
}

long long Config_GetThreadCount() {
	return _Config_GetModuleConfig().thread_count;
}

int Config_GetOMPThreadCount() {
	return _Config_GetModuleConfig().omp_thread_count;
}

uint64_t Config_GetVirtualKeyEntityCount() {
	return _Config_GetModuleConfig().vkey_entity_count;
}

