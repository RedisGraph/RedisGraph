/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "commands.h"
#include "../globals.h"
#include "redismodule.h"
#include "cmd_context.h"
#include "../util/thpool/pools.h"

#include <ctype.h>
#include <string.h>

#define QUERY_KEY_NAME              "Query"
#define GRAPH_NAME_KEY_NAME         "Graph name"
#define TOTAL_DURATION_KEY_NAME     "Total duration"
#define REPLICATION_KEY_NAME        "Replicated command"
#define RECEIVED_TIMESTAMP_KEY_NAME "Received at"

#define SUBCOMMAND_NAME_QUERIES "QUERIES"
#define UNKNOWN_SECTION_MESSAGE "Unknown section."
#define WAIT_DURATION_KEY_NAME "Wait duration"
#define EXECUTION_DURATION_KEY_NAME "Execution duration"

//------------------------------------------------------------------------------
// Info section API
//------------------------------------------------------------------------------

// adds a new section to the info output
static void Info_AddSection
(
	RedisModuleCtx *ctx,  // redis module context
	const char *section,  // section name
	uint32_t n_entries    // number of entries
) {
	// validate arguments
	ASSERT(ctx != NULL);
	ASSERT(n_entries > 0);
	ASSERT(section != NULL);

	RedisModule_ReplyWithArray(ctx, n_entries + 1);
	RedisModule_ReplyWithCString(ctx, section);
}

static void Info_AddSubSection
(
	RedisModuleCtx *ctx,  // redis module context
	const char *section,  // section name
	uint32_t n_entries    // number of entries
) {
	// validate arguments
	ASSERT(ctx != NULL);
	ASSERT(section != NULL);

	RedisModule_ReplyWithArray(ctx, n_entries + 1);
	RedisModule_ReplyWithCString(ctx, section);
}

// adds a new entry to the current section
static void Info_SectionAddEntryString
(
	RedisModuleCtx *ctx,  // redis module context
	const char *entry_name,  // entry name
	const char *entry_value  // entry value
) {
	// validate arguments
	ASSERT(ctx != NULL);
	ASSERT(entry_name != NULL);
	ASSERT(entry_value != NULL);

	RedisModule_ReplyWithCString(ctx, entry_name);
	RedisModule_ReplyWithCString(ctx, entry_value);
}

// adds a new entry to the current section
static void Info_SectionAddEntryLongLong
(
	RedisModuleCtx *ctx,  // redis module context
	const char *entry_name,  // entry name
	long long entry_value    // entry value
) {
	// validate arguments
	ASSERT(ctx != NULL);
	ASSERT(entry_name != NULL);

	RedisModule_ReplyWithCString(ctx, entry_name);
	RedisModule_ReplyWithLongLong(ctx, entry_value);
}

// adds a new entry to the current section
static void Info_SectionAddEntryDouble
(
	RedisModuleCtx *ctx,  // redis module context
	const char *entry_name,  // entry name
	double entry_value       // entry value
) {
	// validate arguments
	ASSERT(ctx != NULL);
	ASSERT(entry_name != NULL);

	RedisModule_ReplyWithCString(ctx, entry_name);
	RedisModule_ReplyWithDouble(ctx, entry_value);
}

// replies with query information
static void _emit_running_query
(
    RedisModuleCtx *ctx,   // redis module context
    const CommandCtx *cmd  // command context
) {
	ASSERT(ctx != NULL);
	ASSERT(cmd != NULL);

	// compute query execution time
    const double total_time = TIMER_GET_ELAPSED_MILLISECONDS(cmd->timer);

	RedisModule_ReplyWithArray(ctx, 5 * 2);

	// emit query received timestamp
	Info_SectionAddEntryLongLong(ctx, RECEIVED_TIMESTAMP_KEY_NAME,
			cmd->received_ts);

	// emit graph name
	Info_SectionAddEntryString(ctx, GRAPH_NAME_KEY_NAME,
			GraphContext_GetName(cmd->graph_ctx));

	// emit query
	Info_SectionAddEntryString(ctx, QUERY_KEY_NAME, cmd->query);

	// emit query wait duration
	Info_SectionAddEntryDouble(ctx, EXECUTION_DURATION_KEY_NAME, total_time);

	// emit rather or not query was replicated
	Info_SectionAddEntryLongLong(ctx, REPLICATION_KEY_NAME,
			cmd->replicated_command);
}

// replies with query information
static void _emit_waiting_query
(
    RedisModuleCtx *ctx,   // redis module context
    const CommandCtx *cmd  // command context
) {
	ASSERT(ctx != NULL);
	ASSERT(cmd != NULL);

	// compute query execution time
    const double total_time = TIMER_GET_ELAPSED_MILLISECONDS(cmd->timer);

	RedisModule_ReplyWithArray(ctx, 5 * 2);

	// emit query received timestamp
	Info_SectionAddEntryLongLong(ctx, RECEIVED_TIMESTAMP_KEY_NAME,
			cmd->received_ts);

	// emit graph name
	Info_SectionAddEntryString(ctx, GRAPH_NAME_KEY_NAME,
			GraphContext_GetName(cmd->graph_ctx));

	// emit query
	Info_SectionAddEntryString(ctx, QUERY_KEY_NAME, cmd->query);

	// emit query execution duration
	Info_SectionAddEntryDouble(ctx, WAIT_DURATION_KEY_NAME, total_time);

	// emit rather or not query was replicated
	Info_SectionAddEntryLongLong(ctx, REPLICATION_KEY_NAME,
			cmd->replicated_command);
}

// handles the "GRAPH.INFO QUERIES" section
// "GRAPH.INFO QUERIES"
static void _info_queries
(
    RedisModuleCtx *ctx,       // redis context
    RedisModuleString **argv,  // command arguments
    const int argc             // number of arguments
) {
    // an example for a command and reply:
    // command:
    // GRAPH.INFO QUERIES
    // reply:
    // "Queries"
	//     "Query id"
    //     "Received at"
    //     "Graph name"
    //     "Query"
    //     "Total duration"

    ASSERT(ctx != NULL);

	//--------------------------------------------------------------------------
	// collect running queries
	//--------------------------------------------------------------------------

	// get all currently executing commands
	// #readers + #writers + Redis main thread
	uint32_t n = ThreadPools_ThreadCount() + 1;
	CommandCtx* cmds[n];
	Globals_GetCommandCtxs(cmds, &n);

	// create a new section in the reply
	Info_AddSection(ctx, "Queries", 2);

	// TODO: support only GRAPH.INFO RunningQueries and GRAPH.INFO WaitingQueries

	// create a new subsection in the reply
	Info_AddSubSection(ctx, "# Current queries", n);

	for(uint32_t i = 0; i < n; i++) {
		CommandCtx *cmd = cmds[i];
		ASSERT(cmd != NULL);

		// emit the command
		_emit_running_query(ctx, cmd);

		// decrease the command's ref count
		// free the command if it's no longer referenced
		CommandCtx_Free(cmd);
	}

	//--------------------------------------------------------------------------
	// collect waiting queries
	//--------------------------------------------------------------------------

	CommandCtx **cmds_ctx = (CommandCtx**)ThreadPools_GetTasksByHandler(Graph_Query,
			(void(*)(void *))CommandCtx_Incref, &n);

	// create a new subsection in the reply
	Info_AddSubSection(ctx, "# Waiting queries", n);

	for(uint32_t i = 0; i < n; i++) {
		CommandCtx *cmd = cmds_ctx[i];
		ASSERT(cmd != NULL);

		// emit the command
		_emit_waiting_query(ctx, cmd);

		// decrease the command's ref count
		// free the command if it's no longer referenced
		CommandCtx_Free(cmd);
	}

	free(cmds_ctx);
}

// attempts to find the specified subcommand of "GRAPH.INFO" and dispatch it
static void _handle_subcommand
(
    RedisModuleCtx *ctx,       // redis module context
    RedisModuleString **argv,  // command arguments
    const int argc,            // number of arguments
    const char *subcmd         // sub command
) {
    ASSERT(ctx    != NULL);
    ASSERT(argv   != NULL);
    ASSERT(subcmd != NULL);

	// TODO: collect all sections and reply with top level array
	// of size n sections...

    if (!strcasecmp(subcmd, SUBCOMMAND_NAME_QUERIES)) {
		// GRAPH.INFO QUERIES
        _info_queries(ctx, argv, argc);
	} else {
        RedisModule_ReplyWithError(ctx, UNKNOWN_SECTION_MESSAGE);
    }
}

// graph.info command handler
// GRAPH.INFO [Section [Section ...]]
// GRAPH.INFO QUERIES
int Graph_Info
(
    RedisModuleCtx *ctx,       // redis module context
    RedisModuleString **argv,  // command arguments
    const int argc             // number of arguments
) {
    ASSERT(ctx != NULL);

	// expecting at least two arguments
    if (argc < 1) {
        return RedisModule_WrongArity(ctx);
    }

	// TODO: return all sections in the case of GRAPH.INFO

    const char *subcmd = RedisModule_StringPtrLen(argv[1], NULL);
    _handle_subcommand(ctx, argv + 2, argc - 2, subcmd);

    return REDISMODULE_OK;
}

