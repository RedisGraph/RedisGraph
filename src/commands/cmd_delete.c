/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cmd_delete.h"

#include <assert.h>
#include "./cmd_context.h"
#include "../graph/graph.h"
#include "../graph/graphcontext.h"
#include "../query_ctx.h"
#include "../resultset/resultset.h"

extern RedisModuleType *GraphContextRedisModuleType;

/* Delete graph, removing the key from Redis and
 * freeing every resource allocated by the graph. */
void _MGraph_Delete(void *args) {
	QueryCtx_BeginTimer(); // Start deletion timing.

	CommandCtx *dCtx = (CommandCtx *)args;
	RedisModuleCtx *ctx = CommandCtx_GetRedisCtx(dCtx);
	GraphContext *gc = CommandCtx_GetGraphContext(dCtx);
	QueryCtx_SetGraphCtx(gc);

	CommandCtx_ThreadSafeContextLock(dCtx);
	{
		GraphContext_Delete(gc, ctx);
	}
	CommandCtx_ThreadSafeContextUnlock(dCtx);

	char *strElapsed;
	double t = QueryCtx_GetExecutionTime();
	asprintf(&strElapsed, "Graph removed, internal execution time: %.6f milliseconds", t);
	RedisModule_ReplyWithStringBuffer(ctx, strElapsed, strlen(strElapsed));
	free(strElapsed);

	CommandCtx_Free(dCtx);
	QueryCtx_Free(); // Reset the QueryCtx and free its allocations.
}

int MGraph_Delete(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	if(argc != 2) return RedisModule_WrongArity(ctx);

	CommandCtx *context;
	const char *graph_name = RedisModule_StringPtrLen(argv[1], NULL);
	GraphContext *gc = GraphContext_Retrieve(ctx, graph_name, false, false);
	if(!gc) {
		RedisModule_ReplyWithError(ctx, "Graph is either missing or referred key is of a different type.");
		return REDISMODULE_OK;
	}

	/* Determin query execution context
	 * queries issued within a LUA script or multi exec block must
	 * run on Redis main thread, others can run on different threads. */
	int flags = RedisModule_GetContextFlags(ctx);
	// Delete commands should always modify slaves.
	bool is_replicated = false;
	if(flags & (REDISMODULE_CTX_FLAGS_MULTI | REDISMODULE_CTX_FLAGS_LUA)) {
		context = CommandCtx_New(ctx, NULL, gc, NULL, argv, argc, is_replicated);
		_MGraph_Delete(context);
	} else {
		RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
		context = CommandCtx_New(NULL, bc, gc, NULL, argv, argc, is_replicated);
		thpool_add_work(_thpool, _MGraph_Delete, context);
	}

	RedisModule_ReplicateVerbatim(ctx);
	return REDISMODULE_OK;
}
