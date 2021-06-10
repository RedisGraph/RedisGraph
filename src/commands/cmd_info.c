/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "../RG.h"
#include "commands.h"
#include "../version.h"
#include "../redismodule.h"


int Graph_Info(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	ASSERT(ctx != NULL);
    RedisModule_ReplyWithArray(ctx, 2);

    RedisModule_ReplyWithCString(ctx, "RedisGraph Version");
    RedisModule_ReplyWithLongLong(ctx, REDISGRAPH_MODULE_VERSION);

	return REDISMODULE_OK;
}

