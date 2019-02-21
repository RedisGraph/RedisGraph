/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _REDISGRAPH_CONFIG_
#define _REDISGRAPH_CONFIG_

#include "redismodule.h"

#define THREAD_COUNT "THREAD_COUNT" // Config param, number of threads in thread pool

// Tries to fetch number of threads from
// command line arguments if specified
// otherwise returns thread count equals to the number
// of cores available. 
long long Config_GetThreadCount (
    RedisModuleCtx *ctx,
    RedisModuleString **argv,
    int argc
);

#endif
