/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "cmd_context.h"
#include "../util/rmalloc.h"
#include "../util/thpool/pools.h"
#include "../slow_log/slow_log.h"
#include "../util/blocked_client.h"

/* Array with one entry per worker thread
 * keeps track after currently executing commands
 * initialized at module.c accessed via cmd_* and debug.c */
CommandCtx **command_ctxs = NULL;

// create a new command context
CommandCtx *CommandCtx_New
(
	RedisModuleCtx *ctx,           // redis module context
	RedisModuleBlockedClient *bc,  // blocked client
	RedisModuleString *cmd_name,   // command to execute
	RedisModuleString *query,      // query string
	GraphContext *graph_ctx,       // graph context
	ExecutorThread thread,         // which thread executes this command
	bool replicated_command,       // whether this instance was spawned by a replication command
	bool compact,                  // whether this query was issued with the compact flag
	long long timeout,             // the query timeout, if specified
	bool timeout_rw,               // apply timeout on both read and write queries
	uint64_t received_ts,          // command received at this  UNIX timestamp
	simple_timer_t timer           // stopwatch started upon command received
) {
	CommandCtx *context = rm_malloc(sizeof(CommandCtx));

	context->bc                 = bc;
	context->ctx                = ctx;
	context->query              = NULL;
	context->thread             = thread;
	context->compact            = compact;
	context->timeout            = timeout;
	context->graph_ctx          = graph_ctx;
	context->timeout_rw         = timeout_rw;
	context->received_ts        = received_ts;
	context->command_name       = NULL;
	context->replicated_command = replicated_command;

	simple_timer_copy(timer, context->timer);

	if(cmd_name) {
		// make a copy of command name
		const char *command_name = RedisModule_StringPtrLen(cmd_name, NULL);
		context->command_name = rm_strdup(command_name);
	}

	if(query) {
		// make a copy of query
		const char *q = RedisModule_StringPtrLen(query, NULL);
		context->query = rm_strdup(q);
	}

	return context;
}

// place given 'ctx' in 'command_ctxs' at position 'tid'
// representing the current thread
void CommandCtx_TrackCtx(CommandCtx *ctx) {
	ASSERT(ctx != NULL);
	ASSERT(command_ctxs != NULL);

	int tid = ThreadPools_GetThreadID();
	ASSERT(command_ctxs[tid] == NULL);

	// set ctx at the current thread entry
	// CommandCtx_Free will remove it eventually
	command_ctxs[tid] = ctx;

	// reset thread memory consumption to 0 (no memory consumed)
	rm_reset_n_alloced();
}

void CommandCtx_UntrackCtx(CommandCtx *ctx) {
	ASSERT(ctx != NULL);
	ASSERT(command_ctxs != NULL);

	int tid = ThreadPools_GetThreadID();
	if(command_ctxs[tid] == NULL) return; // nothing to clean

	ASSERT(command_ctxs[tid] == ctx);

	// set ctx at the current thread entry
	// CommandCtx_Free will remove it eventually
	command_ctxs[tid] = NULL;
}

RedisModuleCtx *CommandCtx_GetRedisCtx(CommandCtx *command_ctx) {
	ASSERT(command_ctx != NULL);
	// Either we already have a context or block client is set.
	if(command_ctx->ctx) return command_ctx->ctx;

	ASSERT(command_ctx->bc != NULL);
	command_ctx->ctx = RedisModule_GetThreadSafeContext(command_ctx->bc);
	return command_ctx->ctx;
}

RedisModuleBlockedClient *CommandCtx_GetBlockingClient(const CommandCtx *command_ctx) {
	ASSERT(command_ctx != NULL);
	return command_ctx->bc;
}

GraphContext *CommandCtx_GetGraphContext(const CommandCtx *command_ctx) {
	ASSERT(command_ctx != NULL);
	return command_ctx->graph_ctx;
}

const char *CommandCtx_GetCommandName(const CommandCtx *command_ctx) {
	ASSERT(command_ctx != NULL);
	return command_ctx->command_name;
}

const char *CommandCtx_GetQuery(const CommandCtx *command_ctx) {
	ASSERT(command_ctx != NULL);
	return command_ctx->query;
}

void CommandCtx_ThreadSafeContextLock(const CommandCtx *command_ctx) {
	/* Acquire lock only when working with a blocked client
	 * otherwise we're running on Redis main thread,
	 * no need to acquire lock. */
	ASSERT(command_ctx != NULL && command_ctx->ctx != NULL);
	if(command_ctx->bc) RedisModule_ThreadSafeContextLock(command_ctx->ctx);
}

void CommandCtx_ThreadSafeContextUnlock(const CommandCtx *command_ctx) {
	/* Release lock only when working with a blocked client
	 * otherwise we're running on Redis main thread,
	 * no need to release lock. */
	ASSERT(command_ctx != NULL && command_ctx->ctx != NULL);
	if(command_ctx->bc) RedisModule_ThreadSafeContextUnlock(command_ctx->ctx);
}

// copy the stopwatch representing the time the command was received
void CommandCtx_GetTimmer
(
	const CommandCtx *command_ctx,
	simple_timer_t timer
) {
	ASSERT(command_ctx != NULL);

	simple_timer_copy(command_ctx->timer, timer);
}

void CommandCtx_Free(CommandCtx *command_ctx) {
	if(command_ctx->bc) {
		RedisGraph_UnblockClient(command_ctx->bc);
		if(command_ctx->ctx) {
			RedisModule_FreeThreadSafeContext(command_ctx->ctx);
		}
	}

	CommandCtx_UntrackCtx(command_ctx);

	if(command_ctx->query) rm_free(command_ctx->query);
	rm_free(command_ctx->command_name);
	rm_free(command_ctx);
}

