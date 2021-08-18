/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "commands.h"
#include "cmd_context.h"
#include "../util/thpool/pools.h"
#include "../configuration/config.h"

#define GRAPH_SIGNATURE_MISSING -1

// Command handler function pointer.
typedef void(*Command_Handler)(void *args);

// Read configuration flags, returning REDIS_MODULE_ERR if flag parsing failed.
static int _read_flags(RedisModuleString **argv, int argc, bool *compact,
		long long *timeout, uint *graph_signature, char **errmsg) {

	ASSERT(compact);
	ASSERT(timeout);

	// set defaults
	*compact = false;  // verbose
	*graph_signature = GRAPH_SIGNATURE_MISSING;
	Config_Option_get(Config_TIMEOUT, timeout);

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

		if(!strcasecmp(arg, "signature")) {
			long long v = GRAPH_SIGNATURE_MISSING;
			int err = REDISMODULE_ERR;
			if(i < argc - 1) {
				i++; // Set the current argument to the signature value.
				err = RedisModule_StringToLongLong(argv[i], &v);
				*graph_signature = v;
			}

			// Emit error on missing, negative, or non-numeric signature values.
			if(err != REDISMODULE_OK || v < 0 || v > UINT_MAX) {
				asprintf(errmsg, "Failed to parse graph signature value");
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

// Returns false if client provided a graph signature
// which mismatch the current graph signature
static bool _verifyGraphSignature(GraphContext *gc, uint signature) {
	// caller did not specify graph signature
	if(signature == GRAPH_SIGNATURE_MISSING) return true;
	return (GraphContext_GetSignature(gc) == signature);
}

static void _rejectOnSignature(RedisModuleCtx *ctx, uint signature) {
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithError(ctx, "signature mismatch");
	RedisModule_ReplyWithLongLong(ctx, signature);
}

// Return true if the command has a valid number of arguments.
static inline bool _validate_command_arity(GRAPH_Commands cmd, int arity) {
	switch(cmd) {
		case CMD_QUERY:
		case CMD_RO_QUERY:
		case CMD_EXPLAIN:
		case CMD_PROFILE:
			// Expect a command, graph name, a query, and optional config flags.
			return arity >= 3 && arity <= 8;
		case CMD_SLOWLOG:
			// Expect just a command and graph name.
			return arity == 2;
		default:
			ASSERT("encountered unhandled query type" && false);
			return false;
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
			ASSERT(false);
	}
	return NULL;
}

// Convert from string representation to an enum.
static GRAPH_Commands determine_command(const char *cmd_name) {
	if(strcasecmp(cmd_name, "graph.QUERY")    == 0) return CMD_QUERY;
	if(strcasecmp(cmd_name, "graph.RO_QUERY") == 0) return CMD_RO_QUERY;
	if(strcasecmp(cmd_name, "graph.EXPLAIN")  == 0) return CMD_EXPLAIN;
	if(strcasecmp(cmd_name, "graph.PROFILE")  == 0) return CMD_PROFILE;
	if(strcasecmp(cmd_name, "graph.SLOWLOG")  == 0) return CMD_SLOWLOG;

	// we shouldn't reach this point
	ASSERT(false);
	return CMD_UNKNOWN;
}

int CommandDispatch(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	bool compact;
	uint signature;
	long long timeout;
	char *errmsg = NULL;
	CommandCtx *context = NULL;

	RedisModuleString *graph_name = argv[1];
	RedisModuleString *query = (argc > 2) ? argv[2] : NULL;
	const char *command_name = RedisModule_StringPtrLen(argv[0], NULL);
	GRAPH_Commands cmd = determine_command(command_name);

	if(_validate_command_arity(cmd, argc) == false) {
		return RedisModule_WrongArity(ctx);
	}

	// parse additional arguments
	int res = _read_flags(argv, argc, &compact, &timeout, &signature, &errmsg);
	if(res == REDISMODULE_ERR) {
		// emit error and exit if argument parsing failed
		RedisModule_ReplyWithError(ctx, errmsg);
		free(errmsg);
		return REDISMODULE_OK;
	}

	GraphContext *gc = GraphContext_Retrieve(ctx, graph_name, true, true);
	// if GraphContext is null, key access failed and an error been emitted
	if(!gc) return REDISMODULE_ERR;

	// return incase caller provided a mismatched graph signature
	if(!_verifyGraphSignature(gc, signature)) {
		_rejectOnSignature(ctx, GraphContext_GetSignature(gc));
		// release the GraphContext, as we increased its reference count
		// when retrieving it
		GraphContext_Release(gc);
		return REDISMODULE_OK;
	}

	// determin query execution context
	// queries issued within a LUA script or multi exec block must
	// run on Redis main thread, others can run on different threads
	int flags = RedisModule_GetContextFlags(ctx);
	bool is_replicated = RedisModule_GetContextFlags(ctx) & REDISMODULE_CTX_FLAGS_REPLICATED;

	ExecutorThread exec_thread = (flags & (REDISMODULE_CTX_FLAGS_MULTI |
										   REDISMODULE_CTX_FLAGS_LUA   |
										   REDISMODULE_CTX_FLAGS_LOADING)) ?
								 EXEC_THREAD_MAIN : EXEC_THREAD_READER;

	Command_Handler handler = get_command_handler(cmd);
	if(exec_thread == EXEC_THREAD_MAIN) {
		// run query on Redis main thread
		context = CommandCtx_New(ctx, NULL, argv[0], query, gc, exec_thread,
								 is_replicated, compact, timeout);
		handler(context);
	} else {
		// run query on a dedicated thread
		RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL,
				NULL, 0);
		context = CommandCtx_New(NULL, bc, argv[0], query, gc, exec_thread,
								 is_replicated, compact, timeout);

		if(ThreadPools_AddWorkReader(handler, context) == THPOOL_QUEUE_FULL) {
			// report an error once our workers thread pool internal queue
			// is full, this error usually happens when the server is
			// under heavy load and is unable to catch up
			RedisModule_ReplyWithError(ctx, "Max pending queries exceeded");
			// release the GraphContext, as we increased its reference count
			// when retrieving it
			GraphContext_Release(gc);
			CommandCtx_Free(context);
		}
	}

	return REDISMODULE_OK;
}

