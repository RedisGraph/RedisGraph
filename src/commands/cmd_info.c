/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

 // this file contains the implementation of the GRAPH.INFO command

#include "RG.h"
#include "util/arr.h"
#include "util/num.h"
#include "query_ctx.h"
#include "util/module.h"
#include "redismodule.h"
#include "graph/graphcontext.h"
#include "configuration/config.h"

#include <ctype.h>
#include <stdlib.h>

#define QUERY_KEY_NAME "Query"
#define STAGE_KEY_NAME "Stage"
#define ALL_GRAPH_KEYS_MASK "*"
#define SUBCOMMAND_NAME_GET "GET"
#define INFO_GET_MEMORY_ARG "MEM"
#define QUERIES_KEY_NAME "Queries"
#define INFO_GET_COUNTS_ARG "COUNTS"
#define INFO_QUERIES_PREV_ARG "PREV"
#define SUBCOMMAND_NAME_RESET "RESET"
#define INFO_GET_STATISTICS_ARG "STAT"
#define GRAPH_NAME_KEY_NAME "Graph name"
#define SUBCOMMAND_NAME_QUERIES "QUERIES"
#define INFO_QUERIES_CURRENT_ARG "CURRENT"
#define GLOBAL_INFO_KEY_NAME "Global info"
#define WAIT_DURATION_KEY_NAME "Wait duration"
#define TOTAL_DURATION_KEY_NAME "Total duration"
#define RECEIVED_TIMESTAMP_KEY_NAME "Received at"
#define REPORT_DURATION_KEY_NAME "Report duration"
#define UNIMPLEMENTED_ERROR_STRING "Unimplemented"
#define UNKNOWN_SUBCOMMAND_MESSAGE "Unknown subcommand."
#define EXECUTION_DURATION_KEY_NAME "Execution duration"
#define INVALID_PARAMETERS_MESSAGE "Invalid parameters."
#define COMMAND_IS_DISABLED "Info tracking is disabled."
#define ERROR_VALUES_OVERFLOW "Some values have overflown"
#define ERROR_NO_GRAPH_NAME_SPECIFIED "No graph name was specified"
#define ERROR_COULD_NOT_FIND_GRAPH "Couldn't find the specified graph"
#define TOTAL_WAITING_QUERIES_COUNT_KEY_NAME "Total waiting queries count"
#define MAX_QUERY_WAIT_TIME_KEY_NAME "Current maximum query wait duration"
#define TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME "Total executing queries count"
#define TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME "Total reporting queries count"
#define INVALID_COUNT_PARAMETER_FOR_PREV_MESSAGE "\"PREV\": Invalid value for the <count> parameter."

// a duplicate of what is set in config.c
#define MAX_QUERIES_COUNT_DEFAULT 10000

#define CHECKED_ADD_OR_RETURN(lhs, rhs, return_on_error) \
    if (!checked_add_u64(lhs, rhs, &lhs)) { \
        return return_on_error; \
    }

// global array tracking all existing GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

// global info - across all the graphs available in all the shards
typedef struct GlobalInfo {
    millis_t max_query_wait_time;
    uint64_t total_waiting_queries_count;
    uint64_t total_executing_queries_count;
    uint64_t total_reporting_queries_count;
} GlobalInfo;

// Flags for the "GRAPH.INFO QUERIES".
typedef enum InfoQueriesFlag {
    InfoQueriesFlag_NONE = 0,
    InfoQueriesFlag_CURRENT = 1 << 0,
    InfoQueriesFlag_PREV = 1 << 1,
} InfoQueriesFlag;

// The data we need to extract from the callback for the circular buffer which
// we later print out in the reply of the "GRAPH.INFO QUERIES PREV".
// It is used as a "user data" field in the view callback.
typedef struct ViewFinishedQueriesCallbackData {
    RedisModuleCtx *ctx;
    uint64_t actual_elements_count;
    int status;
} ViewFinishedQueriesCallbackData;

// returns true if graph info tracking is enabled
static bool _is_cmd_info_enabled(void) {
    bool enabled = false;
    return Config_Option_get(Config_CMD_INFO, &enabled) && enabled;
}

static uint64_t _info_queries_max_count() {
    uint64_t max_elements_count = 0;

    if (!Config_Option_get(Config_CMD_INFO_MAX_QUERY_COUNT, &max_elements_count)) {
        max_elements_count = MAX_QUERIES_COUNT_DEFAULT;
    }

    return max_elements_count;
}

static bool _collect_queries_info_from_graph
(
    RedisModuleCtx *ctx,
    GraphContext *gc,
    GlobalInfo *global_info
) {
    ASSERT(ctx         != NULL);
    ASSERT(gc          != NULL);
    ASSERT(global_info != NULL);

    uint64_t executing = 0;
    uint64_t reporting = 0;
    uint64_t waiting = Info_GetWaitingQueriesCount(gc->info);
    Info_GetExecutingReportingQueriesCount(gc->info, &executing, &reporting);
    uint64_t max_wait_time = Info_GetMaxQueryWaitTime(gc->info);

    global_info->total_waiting_queries_count = waiting;
	global_info->total_executing_queries_count = executing;
    global_info->total_reporting_queries_count = reporting;
	global_info->max_query_wait_time = MAX(max_wait_time,
			global_info->max_query_wait_time);

    return true;
}

// collects all the global information from all the graphs
static bool _collect_global_info
(
    RedisModuleCtx *ctx,
    GlobalInfo *global_info
) {
    ASSERT(ctx != NULL);
	ASSERT(global_info != NULL);
	ASSERT(graphs_in_keyspace != NULL);

    const uint32_t graphs_count = array_len(graphs_in_keyspace);
    memset(global_info, 0, sizeof(GlobalInfo));

    for (uint32_t i = 0; i < graphs_count; ++i) {
        GraphContext *gc = graphs_in_keyspace[i];
		ASSERT(gc != NULL);

        if (!_collect_queries_info_from_graph(ctx, gc, global_info)) {
            RedisModule_ReplyWithError(ctx, "Error while collecting data.");
            return false;
        }
    }

    return true;
}

// Replies with the global information about the graphs, the output is a part
// of the "GRAPH.INFO QUERIES" with no flags.
static int _reply_global_info
(
    RedisModuleCtx *ctx,
    const GlobalInfo global_info
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    ReplyRecorder recorder REPLY_AUTO_FINISH;
    REDISMODULE_ASSERT(ReplyRecorder_New(&recorder, ctx));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        MAX_QUERY_WAIT_TIME_KEY_NAME,
        global_info.max_query_wait_time
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        TOTAL_WAITING_QUERIES_COUNT_KEY_NAME,
        global_info.total_waiting_queries_count
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME,
        global_info.total_executing_queries_count
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME,
        global_info.total_reporting_queries_count
    ));

    return REDISMODULE_OK;
}

// replies with the global query information
// this is a part of the "GRAPH.INFO QUERIES" information
static int _reply_with_queries_info_global
(
    RedisModuleCtx *ctx
) {
    ASSERT(ctx != NULL);

    GlobalInfo global_info;
    if (!_collect_global_info(ctx, &global_info)) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, GLOBAL_INFO_KEY_NAME));
    REDISMODULE_ASSERT(_reply_global_info(ctx, global_info));

    return REDISMODULE_OK;
}

// Parses out a single "GRAPH.INFO QUERIES" flag.
static InfoQueriesFlag _parse_info_queries_flag_from_string
(
    const char *str
) {
    if (!strcasecmp(str, INFO_QUERIES_CURRENT_ARG)) {
        return InfoQueriesFlag_CURRENT;
    } else if (!strcasecmp(str, INFO_QUERIES_PREV_ARG)) {
        return InfoQueriesFlag_PREV;
    }
    return InfoQueriesFlag_NONE;
}

// parses out the "GRAPH.INFO QUERIES" flags from an array of strings
static InfoQueriesFlag _parse_info_queries_flags_from_args
(
    const RedisModuleString **argv,
    const int argc
) {
    InfoQueriesFlag flags = InfoQueriesFlag_NONE;

    if (argc == 0) {
        return flags;
    }

    int read = 0;
    while(read < argc) {
        const char *arg = RedisModule_StringPtrLen(argv[read], NULL);
        flags |= _parse_info_queries_flag_from_string(arg);
        read++;
    }

    return flags;
}

// Replies with the query information, which is relevant either to the already
// finished queries or currently working.
// This is a part of the "GRAPH.INFO QUERIES" reply.
static int _reply_graph_query_info
(
    RedisModuleCtx *ctx,
    const QueryInfo info
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    const millis_t total_spent_time
        = info.wait_duration
        + info.execution_duration
        + info.report_duration;

    ReplyRecorder recorder REPLY_AUTO_FINISH;
    REDISMODULE_ASSERT(ReplyRecorder_New(&recorder, ctx));
    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        RECEIVED_TIMESTAMP_KEY_NAME,
        info.received_ts
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        STAGE_KEY_NAME,
        (long long)info.stage
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddString(
        &recorder,
        GRAPH_NAME_KEY_NAME,
        info.graph_name
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddString(
        &recorder,
        QUERY_KEY_NAME,
        info.query_string
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        TOTAL_DURATION_KEY_NAME,
        total_spent_time
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        WAIT_DURATION_KEY_NAME,
        info.wait_duration
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        EXECUTION_DURATION_KEY_NAME,
        info.execution_duration
    ));

    REDISMODULE_ASSERT(ReplyRecorder_AddNumber(
        &recorder,
        REPORT_DURATION_KEY_NAME,
        info.report_duration
    ));

    return REDISMODULE_OK;
}

// Replies with the finished queries information.
// This function is used as a callback for the circular buffer viewer.
// This is a part of the "GRAPH.INFO QUERIES PREV" reply.
static void _reply_finished_queries(void *user_data, const void *item) {
    ViewFinishedQueriesCallbackData *data
        = (ViewFinishedQueriesCallbackData*)user_data;
    QueryInfo **finished
        = (QueryInfo **)item;

    ASSERT(data != NULL);
    ASSERT(item != NULL);

    const QueryInfo info = **finished;

    int res = _reply_graph_query_info(data->ctx, info);
    ASSERT(res == REDISMODULE_OK);
    UNUSED(res);
}

// replies with the finished queries information
// this is a part of the "GRAPH.INFO QUERIES PREV" reply
static int _reply_with_queries_info_prev
(
    RedisModuleCtx *ctx,
    const uint64_t max_count,
    uint64_t *actual_element_count
) {
    ASSERT(ctx && max_count);
    if (!ctx || !max_count) {
        return REDISMODULE_ERR;
    }

    ViewFinishedQueriesCallbackData user_data = {
        .ctx = ctx,
        .status = REDISMODULE_OK,
        .actual_elements_count = 0
    };

    Info_ViewFinishedQueries(_reply_finished_queries, (void*)&user_data,
        max_count);

    if (actual_element_count) {
        *actual_element_count = user_data.actual_elements_count;
    }

    return user_data.status;
}

// parses and handles the "GRAPH.INFO QUERIES PREV <count>" command
static int _parse_and_reply_info_queries_prev
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    uint64_t *actual_element_count
) {
    ASSERT(ctx  != NULL);
    ASSERT(argv != NULL);
    ASSERT(argc > 0);

    // we expect the count as the last argument
    const char *arg_count = RedisModule_StringPtrLen(argv[argc - 1], NULL);
    if (arg_count && !isdigit(arg_count[0])) {
        RedisModule_ReplyWithError(ctx, INVALID_COUNT_PARAMETER_FOR_PREV_MESSAGE);
        if (actual_element_count) {
            *actual_element_count = 1;
        }
        return REDISMODULE_ERR;
    }

    const long long max_count = MIN(atoll(arg_count), _info_queries_max_count());
    if (max_count > 0) {
        REDISMODULE_ASSERT(_reply_with_queries_info_prev(
            ctx,
            max_count,
            actual_element_count
        ));
    }

    return REDISMODULE_OK;
}

// Updates the query stage timer. This is necessary to do as, at the time the
// "GRAPH.INFO" commands are issued, there might be concurrently running queries
// which have their timer ticking but the counted value not yet updated as it
// hasn't moved to the new stage.
static void _update_query_stage_timer(const QueryStage stage, QueryInfo *info) {
    ASSERT(info != NULL);
    if (!info) {
        return;
    }

    switch (stage) {
        case QueryStage_WAITING: QueryInfo_UpdateWaitingTime(info); break;
        case QueryStage_EXECUTING: QueryInfo_UpdateExecutionTime(info); break;
        case QueryStage_REPORTING: QueryInfo_UpdateReportingTime(info); break;
        default: ASSERT(false); break;
    }
}

// Replies with the information from the QueryInfo storage.
static int _reply_graph_query_info_storage
(
    RedisModuleCtx *ctx,
    const QueryStage query_stage,
    const QueryInfoStorage storage,
    const uint64_t max_count,
    uint64_t *iterated
) {
    ASSERT(ctx && storage);
    if (!ctx || !storage) {
        return REDISMODULE_ERR;
    }

    QueryInfo *qi;
    uint n_queries = array_len(storage);
    uint64_t actual_elements_count = 0;

    for(uint i = 0; i < n_queries; i++) {
        qi = storage[i];
        if (actual_elements_count >= max_count) {
            break;
        } else if (qi->stage != query_stage) {
            continue;
        }

        _update_query_stage_timer(query_stage, qi);
        ++actual_elements_count;
        REDISMODULE_ASSERT(_reply_graph_query_info(
            ctx,
            *qi));
    }

    if (iterated) {
        *iterated = actual_elements_count;
    }

    return REDISMODULE_OK;
}

// Replies with queries information which are currently in the passed stage.
// This is a part of the "GRAPH.INFO QUERIES CURRENT" reply.
static int _reply_with_graph_queries_of_stage
(
    RedisModuleCtx *ctx,
    GraphContext *gc,
    const QueryStage query_stage,
    const uint64_t max_count,
    uint64_t *printed_count
) {
    ASSERT(ctx && gc);
    if (!ctx || !gc) {
        return REDISMODULE_ERR;
    }

    Info *info = gc->info;
    uint64_t iterated = 0;

    QueryInfoStorage storage = array_new(QueryInfo *, 0);
    Info_GetQueries(info, query_stage, &storage);

    if (_reply_graph_query_info_storage(
        ctx,
        query_stage,
        storage,
        max_count,
        &iterated)) {
        return REDISMODULE_ERR;
    }

    if (printed_count) {
        *printed_count = iterated;
    }

    return REDISMODULE_OK;
}

// Replies with all the queries which are in the specified stage and are
// currently working from all the graphs.
// This is a part of the "GRAPH.INFO QUERIES CURRENT" reply.
static int _reply_queries_from_all_graphs
(
    RedisModuleCtx *ctx,
    const QueryStage query_stage,
    const uint64_t max_count,
    uint64_t *printed_count
) {
    ASSERT(ctx && graphs_in_keyspace);
    if (!ctx || !graphs_in_keyspace) {
        return REDISMODULE_ERR;
    }

    const uint32_t graphs_count = array_len(graphs_in_keyspace);

    uint64_t actual_elements_count = 0;

    for (uint32_t i = 0; i < graphs_count; ++i) {
        GraphContext *gc = graphs_in_keyspace[i];
        if (!gc) {
            return REDISMODULE_ERR;
        }

        uint64_t queries_printed = 0;
        if (_reply_with_graph_queries_of_stage(
            ctx,
            gc,
            query_stage,
            max_count,
            &queries_printed)) {
            return REDISMODULE_ERR;
        }
        actual_elements_count += queries_printed;
    }

    if (printed_count) {
        *printed_count = actual_elements_count;
    }

    return REDISMODULE_OK;
}

// Replies with all the queries which are currently working from all the graphs.
// This is a part of the "GRAPH.INFO QUERIES CURRENT" reply.
static int _reply_graph_queries
(
    RedisModuleCtx *ctx,
    const uint64_t max_elements_count,
    uint64_t *actual_elements_count_ptr
) {
    ASSERT(ctx && graphs_in_keyspace && max_elements_count);
    if (!ctx || !graphs_in_keyspace) {
        return REDISMODULE_ERR;
    }
    if (!max_elements_count) {
        return REDISMODULE_OK;
    }

    uint64_t actual_elements_count = 0;
    uint64_t count = 0;
    uint64_t current_limit = max_elements_count;

    // TODO: Was this a mistake? Seems redundant.
    // _reply_queries_from_all_graphs(
    //     ctx,
    //     QueryStage_WAITING,
    //     current_limit,
    //     &count
    // );
    // actual_elements_count += count;
    if (current_limit >= count) {
        current_limit -= count;
    } else {
        current_limit = 0;
    }
    count = 0;

    _reply_queries_from_all_graphs(
        ctx,
        QueryStage_EXECUTING,
        current_limit,
        &count
    );
    actual_elements_count += count;
    if (current_limit >= count) {
        current_limit -= count;
    } else {
        current_limit = 0;
    }
    count = 0;

    _reply_queries_from_all_graphs(
        ctx,
        QueryStage_REPORTING,
        current_limit,
        &count
    );
    actual_elements_count += count;

    if (actual_elements_count_ptr) {
        *actual_elements_count_ptr = actual_elements_count;
    }

    return REDISMODULE_OK;
}

// Replies with the query information from all the graphs.
// This is a part of the "GRAPH.INFO QUERIES" information with either CURRENT or
// PREV flags specified information.
static int _reply_with_queries_info_from_all_graphs
(
    RedisModuleCtx *ctx,
    uint64_t *actual_elements_count
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    const uint64_t max_elements_count = _info_queries_max_count();

    REDISMODULE_ASSERT(_reply_graph_queries(
        ctx,
        max_elements_count,
        actual_elements_count));

    return REDISMODULE_OK;
}

// Parses and handles the "GRAPH.INFO QUERIES" command.
static int _reply_with_queries
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    uint8_t *top_level_count
) {
    ASSERT(ctx != NULL);

    const InfoQueriesFlag flags = _parse_info_queries_flags_from_args(argv,
			argc);

    if (flags == InfoQueriesFlag_NONE) {
        return REDISMODULE_OK;
    }

    REDISMODULE_ASSERT(RedisModule_ReplyWithCString(ctx, QUERIES_KEY_NAME));

    if (*top_level_count) {
        // We count the array below.
        ++*top_level_count;
    }

    REDISMODULE_ASSERT(RedisModule_ReplyWithArray(
        ctx,
        REDISMODULE_POSTPONED_LEN));

    uint64_t actual_element_count = 0;

    if (CHECK_FLAG(flags, InfoQueriesFlag_PREV)) {
        const int ret = _parse_and_reply_info_queries_prev(
            ctx,
            argv,
            argc,
            &actual_element_count
        );

        if (ret != REDISMODULE_OK) {
            RedisModule_ReplySetArrayLength(ctx, actual_element_count);
            return ret;
        }
    }

    if (CHECK_FLAG(flags, InfoQueriesFlag_CURRENT)) {
        uint64_t element_count = 0;
        REDISMODULE_DO(_reply_with_queries_info_from_all_graphs(
            ctx,
            &element_count
        ));
        actual_element_count += element_count;
    }

    RedisModule_ReplySetArrayLength(ctx, actual_element_count);

    return REDISMODULE_OK;
}

// handles the "GRAPH.INFO QUERIES" subcommand
// the format is "GRAPH.INFO QUERIES [CURRENT] [PREV <count>]"
static int _info_queries
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx != NULL);

    uint8_t top_level_count = 1;  // ?

    module_reply_map(ctx, REDISMODULE_POSTPONED_LEN);

	// emit queries statistics
    _reply_with_queries_info_global(ctx);

    const int ret = _reply_with_queries(
        ctx,
        argv,
        argc,
        &top_level_count
    );

    module_reply_map_set_postponed_length(
        ctx,
        top_level_count
    );

    return ret;
}

// attempts to find the specified subcommand of "GRAPH.INFO" and dispatch it
// returns true if the command was found and handled, false otherwise
static bool _dispatch_subcommand
(
    RedisModuleCtx *ctx,             // redis module context
    const RedisModuleString **argv,  // command arguments
    const int argc,                  // number of arguments
    const char *subcmd,              // sub command
    int *result                      // [output] result
) {
    ASSERT(ctx != NULL);
    ASSERT(result != NULL);
    ASSERT(subcmd != NULL);

	// GRAPH.INFO QUERIES
    if (!strcasecmp(subcmd, SUBCOMMAND_NAME_QUERIES)) {
        *result = _info_queries(ctx, argv + 1, argc - 1);
    } else {
		// sub command either un-familiar or not supported
        return false;
    }

    return true;
}

// GRAPH.INFO key
// GRAPH.INFO key RESET
// GRAPH.INFO key STATS
// GRAPH.INFO key QUERIES
// dispatch the subcommand
int Graph_Info
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx);

    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }

    if (_is_cmd_info_enabled() == false) {
        RedisModule_ReplyWithError(ctx, COMMAND_IS_DISABLED);
        return REDISMODULE_ERR;
    }

    int result = REDISMODULE_ERR;

    const char *subcommand_name = RedisModule_StringPtrLen(argv[1], NULL);
    if (!_dispatch_subcommand(
        ctx,
        argv + 1,
        argc - 1,
        subcommand_name,
        &result)) {
        RedisModule_ReplyWithError(ctx, UNKNOWN_SUBCOMMAND_MESSAGE);
    }

    return result;
}

