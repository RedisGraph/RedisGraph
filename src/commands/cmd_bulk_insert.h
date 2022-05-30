/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../redismodule.h"
#include "../util/thpool/thpool.h"

/* Multi threaded bulk insert context. */
typedef struct {
	RedisModuleBlockedClient *bc;   // Blocked client.
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

int Graph_BulkInsert(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

