#include "cmd_context.h"
#include "../util/rmalloc.h"
#include <assert.h>

CommandCtx *CommandCtx_New
(
	RedisModuleCtx *ctx,
	RedisModuleBlockedClient *bc,
	cypher_parse_result_t *parse_result,
	RedisModuleString *graphName,
	RedisModuleString **argv,
	int argc
) {
	CommandCtx *context = rm_malloc(sizeof(CommandCtx));
	context->bc = bc;
	context->ctx = ctx;
	context->parse_result = parse_result;
	context->argv = argv;
	context->argc = argc;
	context->graphName = NULL;

	// Make a copy of graph name.
	if(graphName) context->graphName = rm_strdup(RedisModule_StringPtrLen(graphName, NULL));
	return context;
}

RedisModuleCtx *CommandCtx_GetRedisCtx(CommandCtx *qctx) {
	assert(qctx);
	// Either we already have a context or block client is set.
	if(qctx->ctx) return qctx->ctx;

	assert(qctx->bc);
	qctx->ctx = RedisModule_GetThreadSafeContext(qctx->bc);
	return qctx->ctx;
}

void CommandCtx_ThreadSafeContextLock(const CommandCtx *qctx) {
	/* Acquire lock only when working with a blocked client
	 * otherwise we're running on Redis main thread,
	 * no need to acquire lock. */
	assert(qctx && qctx->ctx);
	if(qctx->bc) RedisModule_ThreadSafeContextLock(qctx->ctx);
}

void CommandCtx_ThreadSafeContextUnlock(const CommandCtx *qctx) {
	/* Release lock only when working with a blocked client
	 * otherwise we're running on Redis main thread,
	 * no need to release lock. */
	assert(qctx && qctx->ctx);
	if(qctx->bc) RedisModule_ThreadSafeContextUnlock(qctx->ctx);
}

void CommandCtx_Free(CommandCtx *qctx) {
	if(qctx->bc) {
		RedisModule_UnblockClient(qctx->bc, NULL);
		RedisModule_FreeThreadSafeContext(qctx->ctx);
	}

	if(qctx->parse_result) cypher_parse_result_free(qctx->parse_result);
	if(qctx->graphName) rm_free(qctx->graphName);
	rm_free(qctx);
}
