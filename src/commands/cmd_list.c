/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "../util/arr.h"
#include "../redismodule.h"
#include "../graph/graphcontext.h"

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

int Graph_List(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	ASSERT(ctx != NULL);
	ASSERT(graphs_in_keyspace != NULL);

	uint count = array_len(graphs_in_keyspace);
	RedisModule_ReplyWithArray(ctx, count);

	// reply with each graph name
	for(uint i = 0; i < count; i ++) {
		GraphContext *gc = graphs_in_keyspace[i];
		const char *name = GraphContext_GetName(gc);
		RedisModule_ReplyWithStringBuffer(ctx, name, strlen(name));
	}

	return REDISMODULE_OK;
}

