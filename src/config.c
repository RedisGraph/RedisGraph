/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "config.h"
#include <unistd.h>
#include <string.h>
#include <assert.h>

long long Config_GetThreadCount(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
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
