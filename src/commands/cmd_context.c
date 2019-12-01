#include "cmd_context.h"
#include "../util/rmalloc.h"
#include <assert.h>

CommandCtx *CommandCtx_New
(
	RedisModuleCtx *ctx,
	RedisModuleBlockedClient *bc,
	const char *command_name,
	GraphContext *graph_ctx,
	RedisModuleString *query,
	RedisModuleString **argv,
	int argc,
	bool replicated_command
) {
	CommandCtx *context = rm_malloc(sizeof(CommandCtx));
	context->bc = bc;
	context->ctx = ctx;
	context->argv = argv;
	context->argc = argc;
	context->replicated_command = replicated_command;
	context->graph_ctx = graph_ctx;
	context->command_name = command_name;

	// Make a copy of query.
	context->query = (query) ? rm_strdup(RedisModule_StringPtrLen(query, NULL)) : NULL;

	return context;
}

RedisModuleCtx *CommandCtx_GetRedisCtx(CommandCtx *command_ctx) {
	assert(command_ctx);
	// Either we already have a context or block client is set.
	if(command_ctx->ctx) return command_ctx->ctx;

	assert(command_ctx->bc);
	command_ctx->ctx = RedisModule_GetThreadSafeContext(command_ctx->bc);
	return command_ctx->ctx;
}

RedisModuleBlockedClient *CommandCtx_GetBlockingClient(const CommandCtx *command_ctx) {
	assert(command_ctx);
	return command_ctx->bc;
}

GraphContext *CommandCtx_GetGraphContext(const CommandCtx *command_ctx) {
	assert(command_ctx);
	return command_ctx->graph_ctx;
}

const char *CommandCtx_GetCommandName(const CommandCtx *command_ctx) {
	assert(command_ctx);
	return command_ctx->command_name;
}

const char *CommandCtx_GetQuery(const CommandCtx *command_ctx) {
	assert(command_ctx);
	return command_ctx->query;
}

void CommandCtx_ThreadSafeContextLock(const CommandCtx *command_ctx) {
	/* Acquire lock only when working with a blocked client
	 * otherwise we're running on Redis main thread,
	 * no need to acquire lock. */
	assert(command_ctx && command_ctx->ctx);
	if(command_ctx->bc) RedisModule_ThreadSafeContextLock(command_ctx->ctx);
}

void CommandCtx_ThreadSafeContextUnlock(const CommandCtx *command_ctx) {
	/* Release lock only when working with a blocked client
	 * otherwise we're running on Redis main thread,
	 * no need to release lock. */
	assert(command_ctx && command_ctx->ctx);
	if(command_ctx->bc) RedisModule_ThreadSafeContextUnlock(command_ctx->ctx);
}

void CommandCtx_Free(CommandCtx *command_ctx) {
	if(command_ctx->bc) {
		RedisModule_UnblockClient(command_ctx->bc, NULL);
		RedisModule_FreeThreadSafeContext(command_ctx->ctx);
	}

	if(command_ctx->query) rm_free(command_ctx->query);
	rm_free(command_ctx);
}

