/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GRAPH_BULK_INSERT_H
#define GRAPH_BULK_INSERT_H

#include "../redismodule.h"
#include "../util/thpool/thpool.h"

extern threadpool _thpool;

/* Multi threaded bulk insert context. */
typedef struct {
	RedisModuleBlockedClient *bc;   // Blocked client.
	double tic[2];                  // timings.
	RedisModuleString **argv;
	int argc;
} BulkInsertContext;

BulkInsertContext *BulkInsertContext_New
(
    RedisModuleCtx *ctx,
    RedisModuleBlockedClient *bc,
    RedisModuleString **argv,
    int argc
);

void BulkInsertContext_Free
(
    BulkInsertContext *ctx
);

int MGraph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif
