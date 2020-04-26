/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "commands.h"
#include "cmd_context.h"
#include <assert.h>
#include <strings.h>

// Command handler function pointer.
typedef void(*Command_Handler)(void *args);

// Return true if the command has a valid number of arguments.
static inline bool _validate_command_arity(GRAPH_Commands cmd, int arity) {
	switch(cmd) {
	case CMD_QUERY:
	case CMD_EXPLAIN:
	case CMD_PROFILE:
		// Expect a command, graph name, a query, and optionally a "--compact" flag.
		return arity >= 3 && arity <= 4;
	case CMD_SLOWLOG:
		// Expect just a command and graph name.
		return arity == 2;
	default:
		assert("encountered unhandled query type" && false);
	}
}

// Get command handler.
static Command_Handler get_command_handler(GRAPH_Commands cmd) {
	switch(cmd) {
	case CMD_QUERY:
		return Graph_Query;
	case CMD_EXPLAIN:
		return Graph_Explain;
	case CMD_PROFILE:
		return Graph_Profile;
	case CMD_SLOWLOG:
		return Graph_Slowlog;
	default:
		assert(false);
	}
	return NULL;
}

// Convert from string representation to an enum.
static GRAPH_Commands determine_command(const char *cmd_name) {
	if(strcasecmp(cmd_name, "graph.QUERY") == 0) return CMD_QUERY;
	if(strcasecmp(cmd_name, "graph.EXPLAIN") == 0) return CMD_EXPLAIN;
	if(strcasecmp(cmd_name, "graph.PROFILE") == 0) return CMD_PROFILE;
	if(strcasecmp(cmd_name, "graph.SLOWLOG") == 0) return CMD_SLOWLOG;

	assert(false);
	return CMD_UNKNOWN;
}

int CommandDispatch(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	CommandCtx *context;

	RedisModuleString *graph_name = argv[1];
	RedisModuleString *query = (argc > 2) ? argv[2] : NULL;
	const char *command_name = RedisModule_StringPtrLen(argv[0], NULL);
	GRAPH_Commands cmd = determine_command(command_name);
	if(_validate_command_arity(cmd, argc) == false) return RedisModule_WrongArity(ctx);
	Command_Handler handler = get_command_handler(cmd);
	GraphContext *gc = GraphContext_Retrieve(ctx, graph_name, true, true);
	// If the GraphContext is null, key access failed and an error has been emitted.
	if(!gc) return REDISMODULE_ERR;

	/* Determin query execution context
	 * queries issued within a LUA script or multi exec block must
	 * run on Redis main thread, others can run on different threads. */
	int flags = RedisModule_GetContextFlags(ctx);
	bool is_replicated = RedisModule_GetContextFlags(ctx) & REDISMODULE_CTX_FLAGS_REPLICATED;
	bool execute_on_main_thread = (flags & (REDISMODULE_CTX_FLAGS_MULTI |
											REDISMODULE_CTX_FLAGS_LUA |
											REDISMODULE_CTX_FLAGS_LOADING));
	if(execute_on_main_thread) {
		// Run query on Redis main thread.
		context = CommandCtx_New(ctx, NULL, argv[0], query, argc, argv, gc, is_replicated);
		handler(context);
	} else {
		// Run query on a dedicated thread.
		RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
		context = CommandCtx_New(NULL, bc, argv[0], query, argc, argv, gc, is_replicated);
		thpool_add_work(_thpool, handler, context);
	}

	return REDISMODULE_OK;
}

