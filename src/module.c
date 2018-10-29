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
#include "graph/graph_type.h"
#include "index/index_type.h"
#include "stores/store_type.h"
#include "util/thpool/thpool.h"
#include "arithmetic/agg_funcs.h"

/* Thread pool. */
threadpool _thpool = NULL;

/* Read Write lock */
pthread_rwlock_t _rwlock;

/* _writelocked is true if the read-write lock was acquired by a writer */
bool _writelocked;

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
    if(StoreType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register storetype\n");
        return REDISMODULE_ERR;
    }

    if(GraphType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register graphtype\n");
        return REDISMODULE_ERR;
    }

    if(IndexType_Register(ctx) == REDISMODULE_ERR) {
        printf("Failed to register indextype\n");
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

    // Initialize read write lock.
    if (pthread_rwlock_init(&_rwlock, NULL)) return REDISMODULE_ERR;
    _writelocked = false;

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
