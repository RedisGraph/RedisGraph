/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _REDISGRAPH_CONFIG_
#define _REDISGRAPH_CONFIG_

#include "redismodule.h"

#define THREAD_COUNT "THREAD_COUNT" // Config param, number of threads in thread pool
#define ENTITIES_THRESHOLD "ENTITIES_THRESHOLD" // Config param, number of entities in virtual key
#define ENTITIES_THRESHOLD_DEFAULT 100000

// Tries to fetch number of threads from
// command line arguments if specified
// otherwise returns thread count equals to the number
// of cores available.
long long Config_GetThreadCount(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc
);

// Tries to fetch the number of entities to encode as part of virtual key encoding.
// Defaults to ENTITIES_THRESHOLD_DEFAULT
uint64_t Config_GetEntitiesThreshold(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc
);

#endif
