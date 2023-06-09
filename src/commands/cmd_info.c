/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
//#include "util/arr.h"
//#include "query_ctx.h"
#include "../globals.h"
#include "redismodule.h"
#include "cmd_context.h"
//#include "graph/graphcontext.h"
//#include "configuration/config.h"

#include <ctype.h>
#include <string.h>

#define QUERY_KEY_NAME              "Query"
#define GRAPH_NAME_KEY_NAME         "Graph name"
#define TOTAL_DURATION_KEY_NAME     "Total duration"
#define REPLICATION_KEY_NAME        "Replicated command"
#define RECEIVED_TIMESTAMP_KEY_NAME "Received at"

//#define STAGE_KEY_NAME "Stage"
//#define ALL_GRAPH_KEYS_MASK "*"
//#define QUERIES_KEY_NAME "Queries"
//#define INFO_GET_COUNTS_ARG "COUNTS"
//
//#define INFO_QUERIES_PREV_ARG "PREV"
//#define INFO_QUERIES_CURRENT_ARG "CURRENT"


//#define SUBCOMMAND_NAME_STATS "STATS"
#define SUBCOMMAND_NAME_QUERIES "QUERIES"

//#define WAIT_DURATION_KEY_NAME "Wait duration"
//#define REPORT_DURATION_KEY_NAME "Report duration"
//#define UNIMPLEMENTED_ERROR_STRING "Unimplemented"
#define UNKNOWN_SUBCOMMAND_MESSAGE "Unknown subcommand."
//#define EXECUTION_DURATION_KEY_NAME "Execution duration"
//#define COMMAND_IS_DISABLED "Info tracking is disabled."
//#define TOTAL_WAITING_QUERIES_COUNT_KEY_NAME "Total waiting queries count"
//#define MAX_QUERY_WAIT_TIME_KEY_NAME "Current maximum query wait duration"
//#define TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME "Total executing queries count"
//#define TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME "Total reporting queries count"
//#define INVALID_COUNT_PARAMETER_FOR_PREV_MESSAGE "\"PREV\": Invalid value for the <count> parameter."

//// global info - across all the graphs available in all the shards
//typedef struct GlobalInfo {
//    double max_query_wait_time;
//    uint64_t total_waiting_queries_count;
//    uint64_t total_executing_queries_count;
//    uint64_t total_reporting_queries_count;
//} GlobalInfo;
//
//// returns true if graph info tracking is enabled
//static bool _is_cmd_info_enabled(void) {
//    bool enabled = false;
//    return Config_Option_get(Config_CMD_INFO, &enabled) && enabled;
//}
//
//// updates the query stage timer
//// this is necessary to do as, at the time the "GRAPH.INFO" commands are issued
//// there might be concurrently running queries which have their timer ticking
//// but the counted value not yet updated as it hasn't moved to the new stage
//static void _update_query_stage_timer
//(
//	QueryInfo *info
//) {
//    ASSERT(info != NULL);
//
//    switch (info->stage) {
//        case QueryStage_WAITING: QueryInfo_UpdateWaitingTime(info); break;
//        case QueryStage_EXECUTING: QueryInfo_UpdateExecutionTime(info); break;
//        case QueryStage_REPORTING: QueryInfo_UpdateReportingTime(info); break;
//        default: ASSERT(false); break;
//    }
//}
//

// replies with query information
static void _emit_query
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
	RedisModule_ReplyWithCString(ctx, RECEIVED_TIMESTAMP_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, cmd->received_ts);

	// emit graph name
	RedisModule_ReplyWithCString(ctx, GRAPH_NAME_KEY_NAME);
	RedisModule_ReplyWithCString(ctx, GraphContext_GetName(cmd->graph_ctx));

	// emit query
	RedisModule_ReplyWithCString(ctx, QUERY_KEY_NAME);
	RedisModule_ReplyWithCString(ctx, cmd->query);

	// emit query execution duration
	RedisModule_ReplyWithCString(ctx, TOTAL_DURATION_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, total_time);

	// emit rather or not query was replicated
	RedisModule_ReplyWithCString(ctx, REPLICATION_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, cmd->replicated_command);
}

//
//// emit queries
//static void _emit_queries
//(
//    RedisModuleCtx *ctx,  // redis module context
//    QueryInfo **queries,  // queries to emit
//	uint n                // number of queries to emit
//) {
//    ASSERT(queries != NULL);
//
//	RedisModule_ReplyWithArray(ctx, n);
//
//    for(uint i = 0; i < n; i++) {
//        QueryInfo *qi = queries[i];
//        _update_query_stage_timer(qi);
//        _emit_query(ctx, qi);
//    }
//}
//
//static void _collect_queries_info_from_graph
//(
//    RedisModuleCtx *ctx,
//    GraphContext *gc,
//    GlobalInfo *global_info
//) {
//    ASSERT(ctx         != NULL);
//    ASSERT(gc          != NULL);
//    ASSERT(global_info != NULL);
//
//    uint64_t waiting       = Info_GetWaitingCount(gc->info);
//    uint64_t executing     = 0;
//    uint64_t reporting     = 0;
//    uint64_t max_wait_time = Info_GetMaxWaitTime(gc->info);
//
//    Info_GetExecutingCount(gc->info, &executing, &reporting);
//
//	global_info->total_waiting_queries_count   += waiting;
//	global_info->total_executing_queries_count += executing;
//	global_info->total_reporting_queries_count += reporting;
//
//	global_info->max_query_wait_time = MAX(max_wait_time,
//			global_info->max_query_wait_time);
//}
//
//// handles the "GRAPH.INFO STATS" subcommand
//static void _info_stats
//(
//    RedisModuleCtx *ctx,
//    const RedisModuleString **argv,
//    const int argc
//) {
//	// emit queries statistics
//    ASSERT(ctx != NULL);
//	ASSERT(graphs_in_keyspace != NULL);
//
//    // an example for a command and reply:
//    // command:
//    // GRAPH.INFO STATS
//    // reply:
//    // "Stats"
//    //     "Current maximum query wait duration"
//    //     "Total waiting queries count"
//    //     "Total executing queries count"
//    //     "Total reporting queries count"
//
//	if(argc != 0) {
//		RedisModule_WrongArity(ctx);
//		return;
//	}
//
//	//--------------------------------------------------------------------------
//	// collects global information
//	//--------------------------------------------------------------------------
//
//    GlobalInfo gi = {0};
//
//    const uint32_t graphs_count = array_len(graphs_in_keyspace);
//    for (uint32_t i = 0; i < graphs_count; ++i) {
//        GraphContext *gc = graphs_in_keyspace[i];
//		ASSERT(gc != NULL);
//
//        _collect_queries_info_from_graph(ctx, gc, &gi);
//    }
//
//	// TODO: not sure we we want to emit this string
//    RedisModule_ReplyWithCString(ctx, "Stats");
//
//	//--------------------------------------------------------------------------
//	// emit
//	//--------------------------------------------------------------------------
//
//	// replies with the global information about the graphs
//	RedisModule_ReplyWithArray(ctx, 4 * 2);
//
//	RedisModule_ReplyWithCString(ctx, MAX_QUERY_WAIT_TIME_KEY_NAME);
//	RedisModule_ReplyWithLongLong(ctx, gi.max_query_wait_time);
//
//	RedisModule_ReplyWithCString(ctx, TOTAL_WAITING_QUERIES_COUNT_KEY_NAME);
//	RedisModule_ReplyWithLongLong(ctx, gi.total_waiting_queries_count);
//
//	RedisModule_ReplyWithCString(ctx, TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME);
//	RedisModule_ReplyWithLongLong(ctx, gi.total_executing_queries_count);
//
//	RedisModule_ReplyWithCString(ctx, TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME);
//	RedisModule_ReplyWithLongLong(ctx, gi.total_reporting_queries_count);
//}
//
//// handles GRAPH.INFO QUERIES PREV <count>
//static void _info_queries_prev
//(
//	RedisModuleCtx *ctx,
//	int prev
//) {
//	// TODO: get circular buffer size from configuration
//	QueryInfo **qs = array_new(QueryInfo*, prev);
//
//	//--------------------------------------------------------------------------
//	// collect queries
//	//--------------------------------------------------------------------------
//
//	Info_GetQueries(NULL, QueryStage_FINISHED, &qs, prev);
//
//	//--------------------------------------------------------------------------
//	// emit queries
//	//--------------------------------------------------------------------------
//
//	uint32_t n = array_len(qs);
//	_emit_queries(ctx, qs, n);
//
//	//--------------------------------------------------------------------------
//	// clean-up
//	//--------------------------------------------------------------------------
//
//	for (uint32_t i = 0; i < n; i++) {
//		QueryInfo_Free(qs[i]);
//	}
//	array_free(qs);
//}

// handles the "GRAPH.INFO QUERIES" subcommand
// "GRAPH.INFO QUERIES"
static void _info_queries
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
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

	if(argc != 0) {
		RedisModule_WrongArity(ctx);
		return;
	}

	// get all currently executing commands
	CommandCtx **commands = Globals_GetCommandCtxs();
	uint32_t n = array_len(commands);

	// emmit commands
	RedisModule_ReplyWithArray(ctx, n);

	for(uint32_t i = 0; i < n; i++) {
		CommandCtx *cmd = commands[i];
		ASSERT(cmd != NULL);

		// emit the command
		_emit_query(ctx, cmd);

		// decrease the command's ref count
		// free the command if it's no longer referenced
		CommandCtx_Free(cmd);
	}

	array_free(commands);
}

// attempts to find the specified subcommand of "GRAPH.INFO" and dispatch it
static void _handle_subcommand
(
    RedisModuleCtx *ctx,             // redis module context
    const RedisModuleString **argv,  // command arguments
    const int argc,                  // number of arguments
    const char *subcmd               // sub command
) {
    ASSERT(ctx    != NULL);
    ASSERT(argv   != NULL);
    ASSERT(subcmd != NULL);

    if (!strcasecmp(subcmd, SUBCOMMAND_NAME_QUERIES)) {
		// GRAPH.INFO QUERIES
        _info_queries(ctx, argv, argc);
	} else {
        RedisModule_ReplyWithError(ctx, UNKNOWN_SUBCOMMAND_MESSAGE);
    }
}

// graph.info command handler
// GRAPH.INFO QUERIES
int Graph_Info
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx != NULL);

	// expecting at least two arguments
    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }

    const char *subcmd = RedisModule_StringPtrLen(argv[1], NULL);
    _handle_subcommand(ctx, argv + 2, argc - 2, subcmd);

    return REDISMODULE_OK;
}

