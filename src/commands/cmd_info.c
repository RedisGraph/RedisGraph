/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "util/arr.h"
#include "query_ctx.h"
#include "redismodule.h"
#include "util/thpool/pools.h"
#include "graph/graphcontext.h"
#include "configuration/config.h"

#include <ctype.h>
#include <stdlib.h>

#define QUERY_KEY_NAME "Query"
#define STAGE_KEY_NAME "Stage"
#define ALL_GRAPH_KEYS_MASK "*"
#define QUERIES_KEY_NAME "Queries"
#define INFO_GET_COUNTS_ARG "COUNTS"

#define INFO_QUERIES_PREV_ARG "PREV"
#define INFO_QUERIES_CURRENT_ARG "CURRENT"

#define UTILIZED_CACHE "Utilized cache"
#define GRAPH_NAME_KEY_NAME "Graph name"

#define SUBCOMMAND_NAME_STATS "STATS"
#define SUBCOMMAND_NAME_QUERIES "QUERIES"

#define WAIT_DURATION_KEY_NAME "Wait duration"
#define TOTAL_DURATION_KEY_NAME "Total duration"
#define RECEIVED_TIMESTAMP_KEY_NAME "Received at"
#define REPORT_DURATION_KEY_NAME "Report duration"
#define UNIMPLEMENTED_ERROR_STRING "Unimplemented"
#define UNKNOWN_SUBCOMMAND_MESSAGE "Unknown subcommand."
#define EXECUTION_DURATION_KEY_NAME "Execution duration"
#define COMMAND_IS_DISABLED "Info tracking is disabled."
#define TOTAL_WAITING_QUERIES_COUNT_KEY_NAME "Total waiting queries count"
#define MAX_QUERY_WAIT_TIME_KEY_NAME "Current maximum query wait duration"
#define TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME "Total executing queries count"
#define TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME "Total reporting queries count"
#define INVALID_COUNT_PARAMETER_FOR_PREV_MESSAGE "\"PREV\": Invalid value for the <count> parameter."

// global array tracking all existing GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

// global info - across all the graphs available in all the shards
typedef struct GlobalInfo {
    millis_t max_query_wait_time;
    uint64_t total_waiting_queries_count;
    uint64_t total_executing_queries_count;
    uint64_t total_reporting_queries_count;
} GlobalInfo;

// returns true if graph info tracking is enabled
static bool _is_cmd_info_enabled(void) {
    bool enabled = false;
    return Config_Option_get(Config_CMD_INFO, &enabled) && enabled;
}

// updates the query stage timer
// this is necessary to do as, at the time the "GRAPH.INFO" commands are issued
// there might be concurrently running queries which have their timer ticking
// but the counted value not yet updated as it hasn't moved to the new stage
static void _update_query_stage_timer
(
	QueryInfo *info
) {
    ASSERT(info != NULL);

    switch (info->stage) {
        case QueryStage_WAITING: QueryInfo_UpdateWaitingTime(info); break;
        case QueryStage_EXECUTING: QueryInfo_UpdateExecutionTime(info); break;
        case QueryStage_REPORTING: QueryInfo_UpdateReportingTime(info); break;
        default: ASSERT(false); break;
    }
}

// replies with the query information
// this is a part of the "GRAPH.INFO QUERIES" reply
static void _emit_query
(
    RedisModuleCtx *ctx,   // redis module context
    const QueryInfo *info  // query info to emit
) {
    const millis_t total_spent_time = info->wait_duration      +
									  info->execution_duration +
									  info->report_duration;

	RedisModule_ReplyWithArray(ctx, 9 * 2);

	RedisModule_ReplyWithCString(ctx, RECEIVED_TIMESTAMP_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, info->received_ts);

	RedisModule_ReplyWithCString(ctx, STAGE_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, info->stage);

	RedisModule_ReplyWithCString(ctx, GRAPH_NAME_KEY_NAME);
	RedisModule_ReplyWithCString(ctx, info->graph_name);

	RedisModule_ReplyWithCString(ctx, QUERY_KEY_NAME);
	RedisModule_ReplyWithCString(ctx, info->query_string);

	RedisModule_ReplyWithCString(ctx, TOTAL_DURATION_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, total_spent_time);

	RedisModule_ReplyWithCString(ctx, WAIT_DURATION_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, info->wait_duration);

	RedisModule_ReplyWithCString(ctx, EXECUTION_DURATION_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, info->execution_duration);

	RedisModule_ReplyWithCString(ctx, REPORT_DURATION_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, info->report_duration);

	RedisModule_ReplyWithCString(ctx, UTILIZED_CACHE);
	RedisModule_ReplyWithLongLong(ctx, info->utilized_cache);
}

// emit queries
static void _emit_queries
(
    RedisModuleCtx *ctx,  // redis module context
    QueryInfo **queries,  // queries to emit
	uint n                // number of queries to emit
) {
    ASSERT(queries != NULL);

	RedisModule_ReplyWithArray(ctx, n);

    for(uint i = 0; i < n; i++) {
        QueryInfo *qi = queries[i];
        _update_query_stage_timer(qi);
        _emit_query(ctx, qi);
    }
}

static void _collect_queries_info_from_graph
(
    RedisModuleCtx *ctx,
    GraphContext *gc,
    GlobalInfo *global_info
) {
    ASSERT(ctx         != NULL);
    ASSERT(gc          != NULL);
    ASSERT(global_info != NULL);

    uint64_t waiting       = Info_GetWaitingCount(gc->info);
    uint64_t executing     = 0;
    uint64_t reporting     = 0;
    uint64_t max_wait_time = Info_GetMaxWaitTime(gc->info);

    Info_GetExecutingCount(gc->info, &executing, &reporting);

	global_info->total_waiting_queries_count   += waiting;
	global_info->total_executing_queries_count += executing;
	global_info->total_reporting_queries_count += reporting;

	global_info->max_query_wait_time = MAX(max_wait_time,
			global_info->max_query_wait_time);
}

// handles the "GRAPH.INFO STATS" subcommand
static void _info_stats
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
	// emit queries statistics
    ASSERT(ctx != NULL);
	ASSERT(graphs_in_keyspace != NULL);

    // an example for a command and reply:
    // command:
    // GRAPH.INFO STATS
    // reply:
    // "Stats"
    //     "Current maximum query wait duration"
    //     "Total waiting queries count"
    //     "Total executing queries count"
    //     "Total reporting queries count"

	if(argc != 0) {
		RedisModule_WrongArity(ctx);
		return;
	}

	//--------------------------------------------------------------------------
	// collects global information
	//--------------------------------------------------------------------------

    GlobalInfo gi = {0};

    const uint32_t graphs_count = array_len(graphs_in_keyspace);
    for (uint32_t i = 0; i < graphs_count; ++i) {
        GraphContext *gc = graphs_in_keyspace[i];
		ASSERT(gc != NULL);

        _collect_queries_info_from_graph(ctx, gc, &gi);
    }

	// TODO: not sure we we want to emit this string
    RedisModule_ReplyWithCString(ctx, "Stats");

	//--------------------------------------------------------------------------
	// emit
	//--------------------------------------------------------------------------

	// replies with the global information about the graphs
	RedisModule_ReplyWithArray(ctx, 4 * 2);

	RedisModule_ReplyWithCString(ctx, MAX_QUERY_WAIT_TIME_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, gi.max_query_wait_time);

	RedisModule_ReplyWithCString(ctx, TOTAL_WAITING_QUERIES_COUNT_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, gi.total_waiting_queries_count);

	RedisModule_ReplyWithCString(ctx, TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, gi.total_executing_queries_count);

	RedisModule_ReplyWithCString(ctx, TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME);
	RedisModule_ReplyWithLongLong(ctx, gi.total_reporting_queries_count);
}

// handles GRAPH.INFO QUERIES PREV <count>
static void _info_queries_prev
(
	RedisModuleCtx *ctx,
	int prev
) {
	// TODO: get circular buffer size from configuration
	QueryInfo **qs = array_new(QueryInfo*, prev);

	//--------------------------------------------------------------------------
	// collect queries
	//--------------------------------------------------------------------------

	Info_GetQueries(NULL, QueryStage_FINISHED, &qs, prev);

	//--------------------------------------------------------------------------
	// emit queries
	//--------------------------------------------------------------------------

	uint32_t n = array_len(qs);
	_emit_queries(ctx, qs, n);

	//--------------------------------------------------------------------------
	// clean-up
	//--------------------------------------------------------------------------

	for (uint32_t i = 0; i < n; i++) {
		QueryInfo_Free(qs[i]);
	}
	array_free(qs);
}

// handels GRAPH.INFO QUERIES CURRENT
static void _info_queries_current
(
	RedisModuleCtx *ctx
) {
    ASSERT(graphs_in_keyspace != NULL);

	// query stages we're interested in
    QueryStage stages = QueryStage_EXECUTING | QueryStage_REPORTING;

	uint32_t n_threads = ThreadPools_ThreadCount();
	uint32_t n_graphs  = array_len(graphs_in_keyspace);
	QueryInfo **qs     = array_new(QueryInfo*, n_threads);

	//--------------------------------------------------------------------------
	// collect queries
	//--------------------------------------------------------------------------

    for (uint32_t i = 0; i < n_graphs && array_len(qs) < n_threads; i++) {
        GraphContext *gc = graphs_in_keyspace[i];
		ASSERT(gc != NULL);
		Info_GetQueries(gc->info, stages, &qs, n_threads);
	}

	//--------------------------------------------------------------------------
	// emit queries
	//--------------------------------------------------------------------------

	uint32_t n = array_len(qs);
	_emit_queries(ctx, qs, n);

	//--------------------------------------------------------------------------
	// clean-up
	//--------------------------------------------------------------------------

	for (uint32_t i = 0; i < n; i++) {
		QueryInfo_Free(qs[i]);
	}
	array_free(qs);
}

// handles the "GRAPH.INFO QUERIES" subcommand
// "GRAPH.INFO QUERIES CURRENT"
// "GRAPH.INFO QUERIES PREV <count>"
static void _info_queries
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    // an example for a command and reply:
    // command:
    // GRAPH.INFO QUERIES CURRENT
    // reply:
    // "Queries"
    //     "Received at"
    //     "Stage"
    //     "Graph name"
    //     "Query"
    //     "Total duration"
    //     "Wait duration"
    //     "Execution duration"
    //     "Report duration"

    ASSERT(ctx != NULL);

	if(argc <= 0 || argc > 2) {
        RedisModule_WrongArity(ctx);
	}

	// sub-command argument
	const char *arg = RedisModule_StringPtrLen(argv[0], NULL);

	if(strcasecmp(arg, INFO_QUERIES_CURRENT_ARG) == 0) {
		// GRAPH.INFO QUERIES CURRENT
		_info_queries_current(ctx);
	} else if(strcasecmp(arg, INFO_QUERIES_PREV_ARG) == 0) {
		// GRAPH.INFO QUERIES PREV <count>
		// try parsing 'count'
		long long prev;
		if(argc != 2 ||
		   RedisModule_StringToLongLong(argv[1], &prev) != REDISMODULE_OK) {
			RedisModule_ReplyWithError(ctx,
					INVALID_COUNT_PARAMETER_FOR_PREV_MESSAGE);
			return;
		}
		_info_queries_prev(ctx, prev);
	} else {
		// unknown argument
		RedisModule_ReplyWithError(ctx, UNKNOWN_SUBCOMMAND_MESSAGE);
	}
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
    ASSERT(result != NULL);
    ASSERT(subcmd != NULL);

    if (!strcasecmp(subcmd, SUBCOMMAND_NAME_QUERIES)) {
		// GRAPH.INFO QUERIES
        _info_queries(ctx, argv, argc);
    } else if(!strcasecmp(subcmd, SUBCOMMAND_NAME_STATS)) {
		// GRAPH.INFO STATS
        _info_stats(ctx, argv, argc);
		// sub command either un-familiar or not supported
	} else {
        RedisModule_ReplyWithError(ctx, UNKNOWN_SUBCOMMAND_MESSAGE);
    }
}

// graph.info command handler
// GRAPH.INFO QUERIES
// GRAPH.INFO QUERIES CURRENT
// GRAPH.INFO QUERIES PREV num
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

	// make sure info tracking is enabled
    if (_is_cmd_info_enabled() == false) {
        RedisModule_ReplyWithError(ctx, COMMAND_IS_DISABLED);
        return REDISMODULE_OK;
    }

    const char *subcommand_name = RedisModule_StringPtrLen(argv[1], NULL);
    _handle_subcommand(ctx, argv + 2, argc - 2, subcommand_name);

    return REDISMODULE_OK;
}

