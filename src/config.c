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
#define ENTITIES_THRESHOLD "ENTITIES_THRESHOLD" // Config param, number of entities in virtual key
#define ENTITIES_THRESHOLD_DEFAULT 100000
#define ENTITIES_THRESHOLD_UNLIMITED UINT64_MAX

extern RG_Config config; // Global module configuration.
static bool _initialized = false;


// Tries to fetch number of threads from
// command line arguments if specified
// otherwise returns thread count equals to the number
// of cores available
static long long _Config_GetThreadCount(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
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

// Tries to fetch the number of entities to encode as part of virtual key encoding.
// Defaults to ENTITIES_THRESHOLD_DEFAULT
static uint64_t _Config_GetEntitiesThreshold(RedisModuleCtx *ctx, RedisModuleString **argv,
											 int argc) {

	// For redis-server versions below 6.0.0, we will not split the graph to virtual keys.
	if(!Redis_Version_GreaterOrEqual(6, 0, 0)) return ENTITIES_THRESHOLD_UNLIMITED;
	// Default.
	long long threshold = ENTITIES_THRESHOLD_DEFAULT;

	// Entities threshold defined in configuration?
	// Expecting configuration to be in the form of key value pairs.
	if(argc % 2 == 0) {
		// Scan arguments for ENTITIES_THRESHOLD.
		for(int i = 0; i < argc; i += 2) {
			const char *param = RedisModule_StringPtrLen(argv[i], NULL);
			if(strcasecmp(param, ENTITIES_THRESHOLD) == 0) {
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
	config.entities_threshold = _Config_GetEntitiesThreshold(ctx, argv, argc);
	config.thread_count = _Config_GetThreadCount(ctx, argv, argc);
	_initialized = true;
}

RG_Config Config_GetModuleConfig() {
	assert(_initialized);
	return config;
}

