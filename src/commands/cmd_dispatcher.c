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
	// TODO: get number of arguments form command.
	if(argc < 2) return RedisModule_WrongArity(ctx);

	RedisModuleString *graph_name = argv[1];
	RedisModuleString *query = (argc > 2) ? argv[2] : NULL;
	const char *command_name = RedisModule_StringPtrLen(argv[0], NULL);
	GRAPH_Commands cmd = determine_command(command_name);
	Command_Handler handler = get_command_handler(cmd);
	GraphContext *gc = GraphContext_Retrieve(ctx, graph_name, true, true);

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
