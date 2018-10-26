/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <unistd.h>
#define REDISMODULE_EXPERIMENTAL_API    // Required for block client.
#include "redismodule.h"
#include "config.h"
#include "version.h"
#include "commands/commands.h"
#include "graph/serializers/graphcontext_type.h"
#include "util/thpool/thpool.h"
#include "arithmetic/agg_funcs.h"
#include "arithmetic/arithmetic_expression.h"

/* Thread pool. */
threadpool _thpool = NULL;

/* Set up thread pool,
 * number of threads within pool should be
 * the number of available hyperthreads.
 * Returns 1 if thread pool initialized, 0 otherwise. */
int _Setup_ThreadPOOL(int threadCount) {
    // Create thread pool.
    _thpool = thpool_init(threadCount);
    if(_thpool == NULL) return 0;

    return 1;
}

int _RegisterDataTypes(RedisModuleCtx *ctx) {
    if(GraphContextType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register GraphContext type\n");
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    /* TODO: when module unloads call GrB_finalize. */
    assert(GrB_init(GrB_NONBLOCKING) == GrB_SUCCESS);

    if (RedisModule_Init(ctx, "graph", REDISGRAPH_MODULE_VERSION, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    AR_RegisterFuncs();     // Register arithmetic functions.
    Agg_RegisterFuncs();    // Register aggregation functions.

    long long threadCount = Config_GetThreadCount(ctx, argv, argc);
    if (!_Setup_ThreadPOOL(threadCount)) return REDISMODULE_ERR;
    RedisModule_Log(ctx, "notice", "Thread pool created, using %d threads.", threadCount);

    if (RedisModule_Init(ctx, "graph", REDISGRAPH_MODULE_VERSION, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (_RegisterDataTypes(ctx) != REDISMODULE_OK) return REDISMODULE_ERR;

    if(RedisModule_CreateCommand(ctx, "graph.QUERY", MGraph_Query, "write deny-oom deny-script", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.DELETE", MGraph_Delete, "write deny-script", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.EXPLAIN", MGraph_Explain, "write deny-script", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.BULK", MGraph_BulkInsert, "write deny-oom deny-script", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

