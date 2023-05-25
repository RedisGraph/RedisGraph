/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "cmd_context.h"
#include "graph/graph.h"
#include "graph/graphcontext.h"
#include "query_ctx.h"
#include "resultset/resultset.h"

// delete graph, removing the key from Redis and
// freeing every resource allocated by the graph
int Graph_Delete
(
	RedisModuleCtx *ctx,
	RedisModuleString **argv,
	int argc
) {
	if(argc != 2) {
		return RedisModule_WrongArity(ctx);
	}

	int res = REDISMODULE_OK;

	RedisModuleString *graph_name = argv[1];

	// increase ref count
	GraphContext *gc = GraphContext_Retrieve(ctx, graph_name, false, false);

	// if the GraphContext is null
	// key access failed and an error has been emitted
	if(!gc) {
		res = REDISMODULE_ERR;
		goto cleanup;
	}

	// remove graph from keyspace
	RedisModuleKey *key = RedisModule_OpenKey(ctx, graph_name, REDISMODULE_WRITE);

	RedisModule_DeleteKey(key);         // decreases graph ref count
	RedisModule_CloseKey(key);          // free key handle
	GraphContext_DecreaseRefCount(gc);  // decrease graph ref count

	RedisModule_ReplyWithSimpleString(ctx, "OK");

	// delete commands should always modify slaves
	RedisModule_ReplicateVerbatim(ctx);

cleanup:
	return res;
}

