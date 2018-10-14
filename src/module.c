/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <unistd.h>
#define REDISMODULE_EXPERIMENTAL_API    // Required for block client.
#include "redismodule.h"
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

/* Set up thread pool,
 * number of threads within pool should be
 * the number of available hyperthreads.
 * Returns 1 if thread pool initialized, 0 otherwise. */
int _Setup_ThreadPOOL() {
    // Create thread pool.
    int CPUCount = sysconf(_SC_NPROCESSORS_ONLN);
    if(CPUCount == -1) return 0;

    _thpool = thpool_init(CPUCount);
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
    assert(GrB_init(GrB_NONBLOCKING) == GrB_SUCCESS);
    /* TODO: when module unloads call GrB_finalize. */

    AR_RegisterFuncs();     // Register arithmetic functions.
    Agg_RegisterFuncs();    // Register aggregation functions.
    if (!_Setup_ThreadPOOL()) return REDISMODULE_ERR;

    // Initialize read write lock.
    if (pthread_rwlock_init(&_rwlock, NULL)) return REDISMODULE_ERR;

    if (RedisModule_Init(ctx, "graph", REDISGRAPH_MODULE_VERSION, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if (_RegisterDataTypes(ctx) != REDISMODULE_OK) return REDISMODULE_ERR;

    if(RedisModule_CreateCommand(ctx, "graph.QUERY", MGraph_Query, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.DELETE", MGraph_Delete, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.EXPLAIN", MGraph_Explain, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    if(RedisModule_CreateCommand(ctx, "graph.BULK", MGraph_BulkInsert, "write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}
