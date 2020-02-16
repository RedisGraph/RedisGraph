#include "cmd_context.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../slow_log/slow_log.h"
#include <assert.h>

CommandCtx *CommandCtx_New
(
	RedisModuleCtx *ctx,
	RedisModuleBlockedClient *bc,
	RedisModuleString *cmd_name,
	RedisModuleString *query,
	int argc,
	RedisModuleString **argv,
	GraphContext *graph_ctx,
	bool replicated_command
) {
	CommandCtx *context = rm_malloc(sizeof(CommandCtx));
	context->bc = bc;
	context->ctx = ctx;
	context->argv = argv;
	context->argc = argc;
	context->query = NULL;
	context->command_name = NULL;
	context->graph_ctx = graph_ctx;
	context->replicated_command = replicated_command;

	size_t len;
	if(cmd_name) {
		// Make a copy of command name.
		const char *command_name = RedisModule_StringPtrLen(cmd_name, &len);
		context->command_name = rm_malloc(sizeof(char) * len + 1);
		memcpy(context->command_name, command_name, len);
		context->command_name[len] = '\0';
	}

	if(query) {
		// Make a copy of query.
		const char *q = RedisModule_StringPtrLen(query, &len);
		context->query = rm_malloc(sizeof(char) * len + 1);
		memcpy(context->query, q, len);
		context->query[len] = '\0';
	}

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

	// Report command to slowlog.
	if(command_ctx->query) {
		GraphContext *gc = CommandCtx_GetGraphContext(command_ctx);
		SlowLog *slowlog = GraphContext_GetSlowLog(gc);
		SlowLog_Add(slowlog, command_ctx->command_name, command_ctx->query, QueryCtx_GetExecutionTime());
		rm_free(command_ctx->query);
	}

	rm_free(command_ctx->command_name);
	rm_free(command_ctx);
}
