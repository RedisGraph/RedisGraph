/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "cypher-parser.h"
#include "../redismodule.h"
#include "../graph/graphcontext.h"

// ExecutorThread lists the diffrent types of threads in the system
typedef enum {
	EXEC_THREAD_MAIN,    // redis main thread
	EXEC_THREAD_READER,  // read only thread
	EXEC_THREAD_WRITER,  // write only thread
} ExecutorThread;

/* Query context, used for concurent query processing. */
typedef struct {
	char *query;                    // Query string.
	RedisModuleCtx *ctx;            // Redis module context.
	char *command_name;             // Command to execute.
	GraphContext *graph_ctx;        // Graph context.
	RedisModuleBlockedClient *bc;   // Blocked client.
	bool replicated_command;        // Whether this instance was spawned by a replication command.
	bool compact;                   // Whether this query was issued with the compact flag.
	ExecutorThread thread;          // Which thread executes this command
	long long timeout;              // The query timeout, if specified.
	bool timeout_rw;                // Apply timeout on both read and write queries.
} CommandCtx;

// Create a new command context.
CommandCtx *CommandCtx_New
(
	RedisModuleCtx *ctx,            // Redis module context.
	RedisModuleBlockedClient *bc,   // Blocked client.
	RedisModuleString *cmd_name,    // Command to execute.
	RedisModuleString *query,       // Query string.
	GraphContext *graph_ctx,        // Graph context.
	ExecutorThread thread,          // Which thread executes this command
	bool replicated_command,        // Whether this instance was spawned by a replication command.
	bool compact,                   // Whether this query was issued with the compact flag.
	long long timeout,              // The query timeout, if specified.
	bool timeout_rw                 // Apply timeout on both read and write queries.
);

// Tracks given 'ctx' such that in case of a crash we will be able to report
// back all of the currently running commands
void CommandCtx_TrackCtx(CommandCtx *ctx);

// Remove the given CommandCtx from tracking.
void CommandCtx_UntrackCtx(CommandCtx *ctx);

// Get Redis module context
RedisModuleCtx *CommandCtx_GetRedisCtx
(
	CommandCtx *command_ctx
);

// Get blocking client.
RedisModuleBlockedClient *CommandCtx_GetBlockingClient
(
	const CommandCtx *command_ctx
);

// Get GraphContext.
GraphContext *CommandCtx_GetGraphContext
(
	const CommandCtx *command_ctx
);

// Get command name.
const char *CommandCtx_GetCommandName
(
	const CommandCtx *command_ctx
);

const char *CommandCtx_GetQuery
(
	const CommandCtx *command_ctx
);

// Acquire Redis global lock.
void CommandCtx_ThreadSafeContextLock
(
	const CommandCtx *command_ctx
);

// Release Redis global lock.
void CommandCtx_ThreadSafeContextUnlock
(
	const CommandCtx *command_ctx
);

// Free command context.
void CommandCtx_Free
(
	CommandCtx *command_ctx
);
