/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../RG.h"
#include "commands.h"
#include "cmd_context.h"
#include "../RG.h"
#include <assert.h>
#include <strings.h>

#define GRAPH_VERSION_MISSING -1

// Command handler function pointer.
typedef void(*Command_Handler)(void *args);

// Read configuration flags, returning REDIS_MODULE_ERR if flag parsing failed.
static int _read_flags(RedisModuleString **argv, int argc, bool *compact,
		long long *timeout, uint *graph_version, char **errmsg) {

	ASSERT(compact);
	ASSERT(timeout);

	// set defaults
	*timeout = 0;      // no timeout
	*compact = false;  // verbose
	*graph_version = GRAPH_VERSION_MISSING;

	// GRAPH.QUERY <GRAPH_KEY> <QUERY>
	// make sure we've got more than 3 arguments
	if(argc <= 3) return REDISMODULE_OK;

	// scan arguments
	for(int i = 3; i < argc; i++) {
		const char *arg = RedisModule_StringPtrLen(argv[i], NULL);

		// compact result-set
		if(!strcasecmp(arg, "--compact")) {
			*compact = true;
			continue;
		}

		if(!strcasecmp(arg, "version")) {
			long long v = GRAPH_VERSION_MISSING;
			int err = REDISMODULE_ERR;
			if(i < argc - 1) {
				i++; // Set the current argument to the version value.
				err = RedisModule_StringToLongLong(argv[i], &v);
				*graph_version = v;
			}

			// Emit error on missing, negative, or non-numeric version values.
			if(err != REDISMODULE_OK || v < 0 || v > UINT_MAX) {
				asprintf(errmsg, "Failed to parse graph version value");
				return REDISMODULE_ERR;
			}

			continue;
		}

		// query timeout
		if(!strcasecmp(arg, "timeout")) {
			int err = REDISMODULE_ERR;
			if(i < argc - 1) {
				i++; // Set the current argument to the timeout value.
				err = RedisModule_StringToLongLong(argv[i], timeout);
			}

			// Emit error on missing, negative, or non-numeric timeout values.
			if(err != REDISMODULE_OK || *timeout < 0) {
				asprintf(errmsg, "Failed to parse query timeout value");
				return REDISMODULE_ERR;
			}
		}
	}
	return REDISMODULE_OK;
}

// Returns false if client provided graph version mismatch queried graph version
static bool _verifyGraphVersion(GraphContext *gc, uint version) {
	// caller did not specify graph version
	if(version == GRAPH_VERSION_MISSING) return true;
	return (GraphContext_GetVersion(gc) == version);
}

static void _rejectOnVersionMismatch(RedisModuleCtx *ctx, uint version) {
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithError(ctx, "version mismatch");
	RedisModule_ReplyWithLongLong(ctx, version);
}

// Return true if the command has a valid number of arguments.
static inline bool _validate_command_arity(GRAPH_Commands cmd, int arity) {
	switch(cmd) {
	case CMD_QUERY:
	case CMD_RO_QUERY:
	case CMD_EXPLAIN:
	case CMD_PROFILE:
		// Expect a command, graph name, a query, and optional config flags.
		return arity >= 3 && arity <= 6;
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
	case CMD_RO_QUERY:
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
	if(strcasecmp(cmd_name, "graph.RO_QUERY") == 0) return CMD_RO_QUERY;
	if(strcasecmp(cmd_name, "graph.EXPLAIN") == 0) return CMD_EXPLAIN;
	if(strcasecmp(cmd_name, "graph.PROFILE") == 0) return CMD_PROFILE;
	if(strcasecmp(cmd_name, "graph.SLOWLOG") == 0) return CMD_SLOWLOG;

	assert(false);
	return CMD_UNKNOWN;
}

int CommandDispatch(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	char *errmsg;
	bool compact;
	uint version;
	long long timeout;
	CommandCtx *context = NULL;

	RedisModuleString *graph_name = argv[1];
	RedisModuleString *query = (argc > 2) ? argv[2] : NULL;
	const char *command_name = RedisModule_StringPtrLen(argv[0], NULL);
	GRAPH_Commands cmd = determine_command(command_name);

	// Parse additional query arguments.
	int res = _read_flags(argv, argc, &compact, &timeout, &version, &errmsg);

	if(res == REDISMODULE_ERR) {
		// Emit error and exit if argument parsing failed.
		RedisModule_ReplyWithError(ctx, errmsg);
		free(errmsg);
		// The API reference dictates that registered functions should always return OK.
		return REDISMODULE_OK;
	}

	if(_validate_command_arity(cmd, argc) == false) return RedisModule_WrongArity(ctx);

	Command_Handler handler = get_command_handler(cmd);
	GraphContext *gc = GraphContext_Retrieve(ctx, graph_name, true, true);
	// If the GraphContext is null, key access failed and an error has been emitted.
	if(!gc) return REDISMODULE_ERR;

	// return incase caller provided a mismatched graph version
	if(!_verifyGraphVersion(gc, version)) {
		_rejectOnVersionMismatch(ctx, GraphContext_GetVersion(gc));
		GraphContext_Release(gc);
		return REDISMODULE_OK;
	}

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
		context = CommandCtx_New(ctx, NULL, argv[0], query, gc, is_replicated, compact, timeout);
		handler(context);
	} else {
		// Run query on a dedicated thread.
		RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
		context = CommandCtx_New(NULL, bc, argv[0], query, gc, is_replicated, compact, timeout);
		thpool_add_work(_thpool, handler, context);
	}

	return REDISMODULE_OK;
}

