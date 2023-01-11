/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "util/arr.h"
#include "util/num.h"
#include "util/module.h"
#include "redismodule.h"
#include "graph/graphcontext.h"
#include "query_ctx.h"
#include "configuration/config.h"

#include <stdlib.h>

#define SUBCOMMAND_NAME_QUERIES "QUERIES"
#define SUBCOMMAND_NAME_GET "GET"
#define SUBCOMMAND_NAME_RESET "RESET"
#define COMPACT_MODE_OPTION "--compact"
#define UNKNOWN_SUBCOMMAND_MESSAGE "Unknown subcommand."
#define INVALID_PARAMETERS_MESSAGE "Invalid parameters."
#define MAX_QUERY_WAIT_TIME_KEY_NAME "Current maximum query wait duration"
#define TOTAL_WAITING_QUERIES_COUNT_KEY_NAME "Total waiting queries count"
#define TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME "Total executing queries count"
#define TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME "Total reporting queries count"
#define RECEIVED_TIMESTAMP_KEY_NAME "Received at"
#define GRAPH_NAME_KEY_NAME "Graph name"
#define QUERY_KEY_NAME "Query"
#define QUERIES_KEY_NAME "Queries"
#define GLOBAL_INFO_KEY_NAME "Global info"
#define TOTAL_DURATION_KEY_NAME "Total duration"
#define WAIT_DURATION_KEY_NAME "Wait duration"
#define EXECUTION_DURATION_KEY_NAME "Execution duration"
#define REPORT_DURATION_KEY_NAME "Report duration"
#define STAGE_KEY_NAME "Stage"
#define UNIMPLEMENTED_ERROR_STRING "Unimplemented"
#define INFO_GET_MEMORY_ARG "MEM"
#define INFO_GET_COUNTS_ARG "COUNTS"
#define INFO_GET_STATISTICS_ARG "STAT"
#define INFO_QUERIES_CURRENT_ARG "CURRENT"
#define INFO_QUERIES_PREV_ARG "PREV"
#define ERROR_COULD_NOT_FIND_GRAPH "Couldn't find the specified graph"
#define ERROR_NO_GRAPH_NAME_SPECIFIED "No graph name was specified"
#define ERROR_VALUES_OVERFLOW "Some values have overflown"
#define ALL_GRAPH_KEYS_MASK "*"
// A duplicate of what is set in config.c
#define MAX_QUERIES_COUNT_DEFAULT 10000


#define CHECKED_ADD_OR_RETURN(lhs, rhs, return_on_error) \
    if (!checked_add_u64(lhs, rhs, &lhs)) { \
        return return_on_error; \
    }

// Global array tracking all existing GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

// Global info - across all the graphs available in all the shards.
typedef struct GlobalInfo {
    millis_t max_query_wait_time_time;
    uint64_t total_waiting_queries_count;
    uint64_t total_executing_queries_count;
    uint64_t total_reporting_queries_count;
} GlobalInfo;

// A stage a query may be in.
typedef enum QueryStage {
    QueryStage_WAITING = 0,
    QueryStage_EXECUTING,
    QueryStage_REPORTING,
    QueryStage_FINISHED,
} QueryStage;

// Flags for the "GRAPH.INFO QUERIES".
typedef enum InfoQueriesFlag {
    InfoQueriesFlag_NONE = 0,
    InfoQueriesFlag_CURRENT = 1 << 0,
    InfoQueriesFlag_PREV = 1 << 1,
} InfoQueriesFlag;

// Flags for the "GRAPH.INFO GET".
typedef enum InfoGetFlag {
    InfoGetFlag_NONE = 0,
    InfoGetFlag_MEMORY = 1 << 0,
    InfoGetFlag_COUNTS = 1 << 1,
    InfoGetFlag_STATISTICS = 1 << 2,
} InfoGetFlag;

// This is a data structure which has all the GRAPH.INFO GET information
// aggregated from all the graphs in the keyspaces of all the shards.
typedef struct AggregatedGraphGetInfo {
    // General information.
    uint64_t graph_count;
    uint64_t node_count;
    uint64_t relationship_count;
    uint64_t node_label_count;
    uint64_t relationship_type_count;
    uint64_t node_index_count;
    uint64_t relationship_index_count;
    uint64_t node_property_name_count;
    uint64_t edge_property_name_count;

    // COUNTS
    FinishedQueryCounters counters;

    // MEM TODO

    // STAT
    Statistics statistics;
} AggregatedGraphGetInfo;

// Initialise the AggregatedGraphGetInfo object.
static bool _AggregatedGraphGetInfo_New(AggregatedGraphGetInfo *info) {
    ASSERT(info && "Info is not provided.");
    if (!info) {
        return false;
    }
    return Statistics_New(&info->statistics);
}

// The data we need to extract from the callback for the circle buffer which we
// later print out in the reply of the "GRAPH.INFO QUERIES PREV".
typedef struct ViewFinishedQueriesCallbackData {
    RedisModuleCtx *ctx;
    bool is_compact_mode;
    uint64_t max_count;
    int status;
    uint64_t actual_elements_count;
} ViewFinishedQueriesCallbackData;

// The same structure is used for the replies for the finished and non-finished
// queries. The objects of this structure should have a very limited lifetime,
// as it uses pointers to a data it doesn't own.
typedef struct CommonQueryInfo {
    uint64_t received_at_ms;
    QueryStage stage;
    char *graph_name;
    char *query_string;
    millis_t wait_duration;
    millis_t execution_duration;
    millis_t report_duration;
    // TODO memory
} CommonQueryInfo;

static CommonQueryInfo _CommonQueryInfo_FromFinished
(
    const FinishedQueryInfo finished
) {
    const CommonQueryInfo info = {
        .received_at_ms = finished.received_unix_timestamp_milliseconds,
        .stage = QueryStage_FINISHED,
        .graph_name = (char *)finished.graph_name,
        .query_string = (char *)finished.query_string,
        .wait_duration = finished.total_wait_duration,
        .execution_duration = finished.total_execution_duration,
        .report_duration = finished.total_report_duration
    };
    return info;
}

static CommonQueryInfo _CommonQueryInfo_FromUnfinished
(
    const QueryInfo unfinished,
    const QueryStage stage
) {
    const CommonQueryInfo info = {
        .received_at_ms = unfinished.received_unix_timestamp_milliseconds,
        .stage = stage,
        .graph_name = (char *)unfinished.context->gc->graph_name,
        .query_string = (char *)unfinished.context->query_data.query,
        .wait_duration = unfinished.wait_duration,
        .execution_duration = unfinished.execution_duration,
        .report_duration = unfinished.report_duration
    };
    return info;
}

static bool _is_cmd_info_enabled() {
    bool cmd_info_enabled = false;
    return Config_Option_get(Config_CMD_INFO, &cmd_info_enabled) && cmd_info_enabled;
}

static uint64_t _info_queries_max_count() {
    uint64_t max_elements_count = 0;

    if (!Config_Option_get(Config_CMD_INFO_MAX_QUERY_COUNT, &max_elements_count)) {
        max_elements_count = MAX_QUERIES_COUNT_DEFAULT;
    }

    return max_elements_count;
}

// Aggregates the information used for the "GRAPH.INFO GET" from all the graphs
// from all the shards available.
static bool AggregatedGraphGetInfo_AddFromGraphContext
(
    AggregatedGraphGetInfo *info,
    const GraphContext *gc
) {
    ASSERT(info && gc);
    if (!info || !gc) {
        return false;
    }
    const uint64_t node_count
        = Graph_NodeCount(gc->g) - Graph_DeletedNodeCount(gc->g);
    CHECKED_ADD_OR_RETURN(info->node_count, node_count, false);

    const uint64_t relationship_count
        = Graph_EdgeCount(gc->g) - Graph_DeletedEdgeCount(gc->g);
    CHECKED_ADD_OR_RETURN(
        info->relationship_count,
        relationship_count,
        false);

    CHECKED_ADD_OR_RETURN(
        info->node_label_count,
        Graph_LabelTypeCount(gc->g),
        false);

    CHECKED_ADD_OR_RETURN(
        info->relationship_type_count,
        Graph_RelationTypeCount(gc->g),
        false);

    CHECKED_ADD_OR_RETURN(
        info->node_index_count,
        GraphContext_NodeIndexCount(gc),
        false);

    CHECKED_ADD_OR_RETURN(
        info->relationship_index_count,
        GraphContext_EdgeIndexCount(gc),
        false);

    CHECKED_ADD_OR_RETURN(
        info->node_property_name_count,
        GraphContext_AllNodePropertyNamesCount(gc),
        false);

    CHECKED_ADD_OR_RETURN(
        info->edge_property_name_count,
        GraphContext_AllEdgePropertyNamesCount(gc),
        false
    );

    FinishedQueryCounters_Add(&info->counters, Info_GetFinishedQueryCounters(gc->info));
    Statistics_Add(&info->statistics, Info_GetStatistics(gc->info));

    ++info->graph_count;

    return true;
}

// Returns true if the strings are equal (case insensitively).
// NOTE: The strings must have a NULL-character at the end (strlen requirement).
static bool _string_equals_case_insensitive(const char *lhs, const char *rhs) {
    if (strlen(lhs) != strlen(rhs)) {
        return false;
    }
    return !strcasecmp(lhs, rhs);
}

// Parses out a single flag from a string, which is suitable for the
// "GRAPH.INFO GET" command.
static InfoGetFlag _parse_info_get_flag_from_string(const char *str) {
    if (_string_equals_case_insensitive(str, INFO_GET_MEMORY_ARG)) {
        return InfoGetFlag_MEMORY;
    } else if (_string_equals_case_insensitive(str, INFO_GET_COUNTS_ARG)) {
        return InfoGetFlag_COUNTS;
    } else if (_string_equals_case_insensitive(str, INFO_GET_STATISTICS_ARG)) {
        return InfoGetFlag_STATISTICS;
    }
    return InfoGetFlag_NONE;
}

// Parses out all the flags from an array of strings. The flags extracted are to
// be used for the "GRAPH.INFO GET" command.
static InfoGetFlag _parse_info_get_flags_from_args
(
    const RedisModuleString **argv,
    const int argc
) {
    InfoGetFlag flags = InfoGetFlag_NONE;

    if (!argv || argc <= 0) {
        return flags;
    }

    int read = 0;
    while (read < argc) {
        const char *arg = RedisModule_StringPtrLen(argv[read], NULL);
        flags |= _parse_info_get_flag_from_string(arg);
        ++read;
    }

    return flags;
}

// Parses out a single "GRAPH.INFO QUERIES" flag.
static InfoQueriesFlag _parse_info_queries_flag_from_string(const char *str) {
    if (_string_equals_case_insensitive(str, INFO_QUERIES_CURRENT_ARG)) {
        return InfoQueriesFlag_CURRENT;
    } else if (_string_equals_case_insensitive(str, INFO_QUERIES_PREV_ARG)) {
        return InfoQueriesFlag_PREV;
    }
    return InfoQueriesFlag_NONE;
}

// Parses out the "GRAPH.INFO QUERIES" flags from an array of strings.
static InfoQueriesFlag _parse_info_queries_flags_from_args
(
    const RedisModuleString **argv,
    const int argc
) {
    InfoQueriesFlag flags = InfoQueriesFlag_NONE;

    if (!argv || argc <= 0) {
        return flags;
    }

    int read = 0;
    while (read < argc) {
        const char *arg = RedisModule_StringPtrLen(argv[read], NULL);
        flags |= _parse_info_queries_flag_from_string(arg);
        ++read;
    }

    return flags;
}

static bool _is_queries_cmd(const char *cmd) {
    return _string_equals_case_insensitive(cmd, SUBCOMMAND_NAME_QUERIES);
}

static bool _is_get_cmd(const char *cmd) {
    return _string_equals_case_insensitive(cmd, SUBCOMMAND_NAME_GET);
}

static bool _is_reset_cmd(const char *cmd) {
    return _string_equals_case_insensitive(cmd, SUBCOMMAND_NAME_RESET);
}

static bool _is_compact_mode(const char *arg) {
    return _string_equals_case_insensitive(arg, COMPACT_MODE_OPTION);
}

static bool _collect_queries_info_from_graph
(
    RedisModuleCtx *ctx,
    GraphContext *gc,
    GlobalInfo *global_info
) {
    ASSERT(ctx && gc && global_info);
    if (!ctx || !gc || !global_info) {
        return false;
    }

    bool is_ok = true;

    const uint64_t waiting_queries_count = Info_GetWaitingQueriesCount(&gc->info);
    const uint64_t executing_queries_count = Info_GetExecutingQueriesCount(&gc->info);
    const uint64_t reporting_queries_count = Info_GetReportingQueriesCount(&gc->info);
    const uint64_t max_query_wait_time_time = Info_GetMaxQueryWaitTime(&gc->info);

    if (!checked_add_u64(
        global_info->total_waiting_queries_count,
        waiting_queries_count,
        &global_info->total_waiting_queries_count)) {
        // We have a value overflow.
        if (!is_ok) {
            return false;
        }
    }

    if (!checked_add_u64(
        global_info->total_executing_queries_count,
        executing_queries_count,
        &global_info->total_executing_queries_count)) {
        // We have a value overflow.
        if (!is_ok) {
            return false;
        }
    }

    if (!checked_add_u64(
        global_info->total_reporting_queries_count,
        reporting_queries_count,
        &global_info->total_reporting_queries_count)) {
        // We have a value overflow.
        if (!is_ok) {
            return false;
        }
    }

    if (!checked_add_u32(
        global_info->max_query_wait_time_time,
        max_query_wait_time_time,
        &global_info->max_query_wait_time_time)) {
        // We have a value overflow.
        if (!is_ok) {
            return false;
        }
    }

    return true;
}

// Replies with the global information about the graphs, the output is a part
// of the "GRAPH.INFO" with no flags.
static int _reply_global_info
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const GlobalInfo global_info
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    ReplyRecorder recorder REPLY_AUTO_FINISH;
    REDISMODULE_DO(ReplyRecorder_New(&recorder, ctx, is_compact_mode));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        MAX_QUERY_WAIT_TIME_KEY_NAME,
        global_info.max_query_wait_time_time
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        TOTAL_WAITING_QUERIES_COUNT_KEY_NAME,
        global_info.total_waiting_queries_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME,
        global_info.total_executing_queries_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME,
        global_info.total_reporting_queries_count
    ));

    return REDISMODULE_OK;
}

// Returns a GraphContext object of a graph found by name.
static GraphContext* _find_graph_with_name
(
    const char *graph_name
) {
    ASSERT(graph_name);
    if (!graph_name) {
        return NULL;
    }

    const uint32_t graphs_count = array_len(graphs_in_keyspace);

    for (uint32_t i = 0; i < graphs_count; ++i) {
        GraphContext *gc = graphs_in_keyspace[i];
        if (!gc) {
            return NULL;
        }

        if (_string_equals_case_insensitive(graph_name, gc->graph_name)) {
            return gc;
        }
    }

    return NULL;
}

// Collects all the global information from all the graphs.
// TODO also collect from the shards.
static bool _collect_global_info
(
    RedisModuleCtx *ctx,
    GlobalInfo *global_info
) {
    ASSERT(ctx && global_info && graphs_in_keyspace);
    if (!ctx || !global_info || !graphs_in_keyspace) {
        return REDISMODULE_ERR;
    }

    const uint32_t graphs_count = array_len(graphs_in_keyspace);
    memset(global_info, 0, sizeof(GlobalInfo));

    for (uint32_t i = 0; i < graphs_count; ++i) {
        GraphContext *gc = graphs_in_keyspace[i];
        if (!gc) {
            RedisModule_ReplyWithError(ctx, "Graph does not exist.");
            return false;
        }

        if (!_collect_queries_info_from_graph(ctx, gc, global_info)) {
            RedisModule_ReplyWithError(ctx, "Error while collecting data.");
            return false;
        }
    }

    return true;
}

// Updates the query stage timer. This is necessary to do as, at the time the
// "GRAPH.INFO" commands are issued, there might be concurrently running queries
// which have their timer ticking but the counted value not yet updated as it
// hasn't moved to the new stage.
static void _update_query_stage_timer(const QueryStage stage, QueryInfo *info) {
    ASSERT(info);
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

// Replies with the query information, which is relevant either to the already
// finished queries or currently working.
// This is a part of the "GRAPH.INFO QUERIES" reply.
static int _reply_graph_query_info
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const CommonQueryInfo info
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
    REDISMODULE_DO(ReplyRecorder_New(&recorder, ctx, is_compact_mode));
    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        RECEIVED_TIMESTAMP_KEY_NAME,
        info.received_at_ms
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        STAGE_KEY_NAME,
        (long long)info.stage
    ));

    REDISMODULE_DO(ReplyRecorder_AddString(
        &recorder,
        GRAPH_NAME_KEY_NAME,
        info.graph_name
    ));

    REDISMODULE_DO(ReplyRecorder_AddString(
        &recorder,
        QUERY_KEY_NAME,
        info.query_string
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        TOTAL_DURATION_KEY_NAME,
        total_spent_time
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        WAIT_DURATION_KEY_NAME,
        info.wait_duration
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        EXECUTION_DURATION_KEY_NAME,
        info.execution_duration
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        REPORT_DURATION_KEY_NAME,
        info.report_duration
    ));

    return REDISMODULE_OK;
}

// Replies with the information from the QueryInfo storage.
static int _reply_graph_query_info_storage
(
    RedisModuleCtx *ctx,
    const QueryStage query_stage,
    const QueryInfoStorage *storage,
    const bool is_compact_mode,
    const uint64_t max_count,
    uint64_t *iterated
) {
    ASSERT(ctx && storage);
    if (!ctx || !storage) {
        return REDISMODULE_ERR;
    }

    QueryInfoIterator iterator = QueryInfoIterator_New(storage);
    uint64_t actual_elements_count = 0;
    QueryInfo *info = NULL;
    while ((info = QueryInfoIterator_NextValid(&iterator)) != NULL) {
        if (actual_elements_count >= max_count) {
            break;
        }
        _update_query_stage_timer(query_stage, info);
        ++actual_elements_count;
        REDISMODULE_DO(_reply_graph_query_info(
            ctx,
            is_compact_mode,
            _CommonQueryInfo_FromUnfinished(*info, query_stage)));
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
    const bool is_compact_mode,
    GraphContext *gc,
    const QueryStage query_stage,
    const uint64_t max_count,
    uint64_t *printed_count
) {
    ASSERT(ctx && gc);
    if (!ctx || !gc) {
        return REDISMODULE_ERR;
    }

    Info *info = &gc->info;
    uint64_t iterated = 0;
    Info_Lock(info);

    QueryInfoStorage *storage = NULL;
    switch (query_stage) {
        case QueryStage_WAITING: {
            storage = Info_GetWaitingQueriesStorage(info);
            break;
        }
        case QueryStage_EXECUTING: {
            storage = Info_GetExecutingQueriesStorage(info);
            break;
        }
        case QueryStage_REPORTING: {
            storage = Info_GetReportingQueriesStorage(info);
            break;
        }
        default: Info_Unlock(info); return REDISMODULE_ERR;
    }
    if (_reply_graph_query_info_storage(
        ctx,
        query_stage,
        storage,
        is_compact_mode,
        max_count,
        &iterated)) {
        Info_Unlock(info);
        return REDISMODULE_ERR;
    }

    if (printed_count) {
        *printed_count = iterated;
    }

    Info_Unlock(info);
    return REDISMODULE_OK;
}

// Replies with all the queries which are in the specified stage and are
// currently working from all the graphs.
// This is a part of the "GRAPH.INFO QUERIES CURRENT" reply.
static int _reply_queries_from_all_graphs
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
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
            is_compact_mode,
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
    const bool is_compact_mode,
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

    _reply_queries_from_all_graphs(
        ctx,
        is_compact_mode,
        QueryStage_WAITING,
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
        is_compact_mode,
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
        is_compact_mode,
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

// Replies with the global query information.
// This is a part of the "GRAPH.INFO QUERIES" information.
static int _reply_with_queries_info_global
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    GlobalInfo global_info = {};
    if (!_collect_global_info(ctx, &global_info)) {
        return REDISMODULE_ERR;
    }

    if (!is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, GLOBAL_INFO_KEY_NAME));
    }
    REDISMODULE_DO(_reply_global_info(ctx, is_compact_mode, global_info));

    return REDISMODULE_OK;
}

// Replies with the query information from all the graphs.
// This is a part of the "GRAPH.INFO QUERIES" information with either CURRENT or
// PREV flags specified information.
static int _reply_with_queries_info_from_all_graphs
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    uint64_t *actual_elements_count
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    const uint64_t max_elements_count = _info_queries_max_count();

    REDISMODULE_DO(_reply_graph_queries(
        ctx,
        is_compact_mode,
        max_elements_count,
        actual_elements_count));

    return REDISMODULE_OK;
}

// Resets the information gathered so far in all the graphs.
static int _reset_all_graphs_info
(
    RedisModuleCtx *ctx
) {
    ASSERT(ctx && graphs_in_keyspace);
    if (!ctx || !graphs_in_keyspace) {
        return REDISMODULE_ERR;
    }

    const uint32_t graphs_count = array_len(graphs_in_keyspace);

    for (uint32_t i = 0; i < graphs_count; ++i) {
        GraphContext *gc = graphs_in_keyspace[i];
        ASSERT(gc);
        if (!gc) {
            return REDISMODULE_ERR;
        }
        Info_Reset(&gc->info);
    }
    REDISMODULE_DO(RedisModule_ReplyWithBool(ctx, true));
    return REDISMODULE_OK;
}

// Resets the specified graphs information gathered so far.
static int _reset_graph_info
(
    RedisModuleCtx *ctx,
    const char *graph_name
) {
    ASSERT(ctx && graphs_in_keyspace);
    if (!ctx || !graphs_in_keyspace) {
        return REDISMODULE_ERR;
    }

    const uint32_t graphs_count = array_len(graphs_in_keyspace);

    for (uint32_t i = 0; i < graphs_count; ++i) {
        GraphContext *gc = graphs_in_keyspace[i];
        ASSERT(gc);
        if (!gc) {
            return REDISMODULE_ERR;
        }
        if (_string_equals_case_insensitive(graph_name, gc->graph_name)) {
            Info_Reset(&gc->info);
            REDISMODULE_DO(RedisModule_ReplyWithBool(ctx, true));
            return REDISMODULE_OK;
        }
    }
    RedisModule_ReplyWithError(ctx, ERROR_COULD_NOT_FIND_GRAPH);
    return REDISMODULE_ERR;
}

// Replies with an aggregated information over all the graphs.
// This is a part of the "GRAPH.INFO GET *" reply.
static int _reply_with_get_aggregated_graph_info
(
    RedisModuleCtx *ctx,
    const AggregatedGraphGetInfo info,
    const bool is_compact_mode,
    const InfoGetFlag flags
) {
    ASSERT(ctx);

    ReplyRecorder recorder REPLY_AUTO_FINISH;
    REDISMODULE_DO(ReplyRecorder_New(&recorder, ctx, is_compact_mode));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of graphs",
        info.graph_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of nodes",
        info.node_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of relationships",
        info.relationship_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of node labels",
        info.node_label_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of relationship types",
        info.relationship_type_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of node indices",
        info.node_index_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of relationship indices",
        info.relationship_index_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Total number of edge properties",
        info.node_property_name_count
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Total number of node properties",
        info.edge_property_name_count
    ));

    if (CHECK_FLAG(flags, InfoGetFlag_COUNTS)) {
        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Total number of queries",
            FinishedQueryCounters_GetTotalCount(info.counters)
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Successful read-only queries",
            info.counters.readonly_succeeded_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Successful write queries",
            info.counters.write_succeeded_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Failed read-only queries",
            info.counters.readonly_failed_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Failed write queries",
            info.counters.write_failed_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Timed out read-only queries",
            info.counters.readonly_timedout_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Timed out write queries",
            info.counters.write_timedout_count
        ));
    }

    if (CHECK_FLAG(flags, InfoGetFlag_STATISTICS)) {
        const Percentiles percentiles
            = Statistics_GetPercentiles(info.statistics);

        REDISMODULE_DO(ReplyRecorder_AddNumbers(
            &recorder,
            "Query total durations",
            percentiles.total_durations,
            sizeof(percentiles.total_durations) / sizeof(percentiles.total_durations[0])
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumbers(
            &recorder,
            "Query wait durations",
            percentiles.wait_durations,
            sizeof(percentiles.wait_durations) / sizeof(percentiles.wait_durations[0])
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumbers(
            &recorder,
            "Query execution durations",
            percentiles.execution_durations,
            sizeof(percentiles.execution_durations) / sizeof(percentiles.execution_durations[0])
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumbers(
            &recorder,
            "Query report durations",
            percentiles.report_durations,
            sizeof(percentiles.report_durations) / sizeof(percentiles.report_durations[0])
        ));

        // TODO memory
    }

    if (CHECK_FLAG(flags, InfoGetFlag_MEMORY)) {
        REDISMODULE_DO(ReplyRecorder_AddString(
            &recorder,
            "[MEM]",
            "The flag is not implemented."
        ));
    }

    return REDISMODULE_OK;
}

// Replies with an information of the specified graph.
// This is a part of the "GRAPH.INFO GET <key>" reply.
static int _reply_with_get_graph_info
(
    RedisModuleCtx *ctx,
    GraphContext *gc,
    const bool is_compact_mode,
    const InfoGetFlag flags
) {
    ASSERT(ctx && gc && gc->g);

    ReplyRecorder recorder REPLY_AUTO_FINISH;
    REDISMODULE_DO(ReplyRecorder_New(&recorder, ctx, is_compact_mode));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of nodes",
        Graph_NodeCount(gc->g) - Graph_DeletedNodeCount(gc->g)
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of relationships",
        Graph_EdgeCount(gc->g) - Graph_DeletedEdgeCount(gc->g)
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of node labels",
        Graph_LabelTypeCount(gc->g)
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of relationship types",
        Graph_RelationTypeCount(gc->g)
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of node indices",
        GraphContext_NodeIndexCount(gc)
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of relationship indices",
        GraphContext_EdgeIndexCount(gc)
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Number of unique property names",
        GraphContext_AttributeCount(gc)
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Total number of edge properties",
        GraphContext_AllEdgePropertyNamesCount(gc)
    ));

    REDISMODULE_DO(ReplyRecorder_AddNumber(
        &recorder,
        "Total number of node properties",
        GraphContext_AllNodePropertyNamesCount(gc)
    ));

    if (CHECK_FLAG(flags, InfoGetFlag_COUNTS)) {
        const FinishedQueryCounters counters
            = Info_GetFinishedQueryCounters(gc->info);

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Total number of queries",
            FinishedQueryCounters_GetTotalCount(counters)
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Successful read-only queries",
            counters.readonly_succeeded_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Successful write queries",
            counters.write_succeeded_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Failed read-only queries",
            counters.readonly_failed_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Failed write queries",
            counters.write_failed_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Timed out read-only queries",
            counters.readonly_timedout_count
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumber(
            &recorder,
            "Timed out write queries",
            counters.write_timedout_count
        ));
    }

    if (CHECK_FLAG(flags, InfoGetFlag_STATISTICS)) {
        const Statistics statistics
            = Info_GetStatistics(gc->info);
        const Percentiles percentiles
            = Statistics_GetPercentiles(statistics);

        REDISMODULE_DO(ReplyRecorder_AddNumbers(
            &recorder,
            "Query total durations",
            percentiles.total_durations,
            sizeof(percentiles.total_durations) / sizeof(percentiles.total_durations[0])
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumbers(
            &recorder,
            "Query wait durations",
            percentiles.wait_durations,
            sizeof(percentiles.wait_durations) / sizeof(percentiles.wait_durations[0])
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumbers(
            &recorder,
            "Query execution durations",
            percentiles.execution_durations,
            sizeof(percentiles.execution_durations) / sizeof(percentiles.execution_durations[0])
        ));

        REDISMODULE_DO(ReplyRecorder_AddNumbers(
            &recorder,
            "Query report durations",
            percentiles.report_durations,
            sizeof(percentiles.report_durations) / sizeof(percentiles.report_durations[0])
        ));

        // TODO memory
    }

    if (CHECK_FLAG(flags, InfoGetFlag_MEMORY)) {
        REDISMODULE_DO(ReplyRecorder_AddString(
            &recorder,
            "[MEM]",
            "The flag is not implemented."
        ));
    }

    return REDISMODULE_OK;
}

// Handles the specific graph information request.
// This is an action of the "GRAPH.INFO GET <key>" command.
static int _get_graph_info
(
    RedisModuleCtx *ctx,
    const char *graph_name,
    const bool is_compact_mode,
    const InfoGetFlag flags
) {
    ASSERT(ctx && graph_name);

    GraphContext *gc = _find_graph_with_name(graph_name);
    if (!gc) {
        RedisModule_ReplyWithError(ctx, ERROR_COULD_NOT_FIND_GRAPH);
        return REDISMODULE_ERR;
    }

    return _reply_with_get_graph_info(ctx, gc, is_compact_mode, flags);
}

// Handles graph information request regarding all the graphs available.
// This is an action of the "GRAPH.INFO GET *" command.
static int _get_all_graphs_info
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const InfoGetFlag flags
) {
    ASSERT(ctx);
    const uint32_t graphs_count = array_len(graphs_in_keyspace);
    AggregatedGraphGetInfo info = {};
    const bool initialised = _AggregatedGraphGetInfo_New(&info);
    ASSERT(initialised && "Couldn't initialise aggregated info.");
    if (!initialised) {
        RedisModule_ReplyWithError(ctx, "Couldn't initialise aggregated info.");
        return REDISMODULE_ERR;
    }

    for (uint32_t i = 0; i < graphs_count; ++i) {
        const GraphContext *gc = graphs_in_keyspace[i];
        if (!gc) {
            return REDISMODULE_ERR;
        }

        if (!AggregatedGraphGetInfo_AddFromGraphContext(&info, gc)) {
            RedisModule_ReplyWithError(ctx, ERROR_VALUES_OVERFLOW);
            return REDISMODULE_ERR;
        }
    }

    return _reply_with_get_aggregated_graph_info(
        ctx,
        info,
        is_compact_mode,
        flags
    );
}

// Replies with the finished queries information.
// This function is used as a callback for the circle buffer viewer.
// This is a part of the "GRAPH.INFO QUERIES PREV" reply.
static bool _reply_finished_queries(void *user_data, const void *item) {
    ViewFinishedQueriesCallbackData *data
        = (ViewFinishedQueriesCallbackData*)user_data;
    const FinishedQueryInfo *finished
        = (FinishedQueryInfo*)item;

    ASSERT(data);
    ASSERT(item);
    if (!data || !item) {
        data->status = REDISMODULE_ERR;
        return true;
    }

    const CommonQueryInfo info = _CommonQueryInfo_FromFinished(*finished);

    REDISMODULE_DO(_reply_graph_query_info(data->ctx, data->is_compact_mode, info))

    if (++data->actual_elements_count >= data->max_count) {
        return true;
    }

    return false;
}

// Replies with the finished queries information.
// This is a part of the "GRAPH.INFO QUERIES PREV" reply.
static int _reply_with_queries_info_prev
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const uint64_t max_count,
    uint64_t *actual_element_count
) {
    ASSERT(ctx && max_count);
    if (!ctx || !max_count) {
        return REDISMODULE_ERR;
    }

    ViewFinishedQueriesCallbackData user_data = {
        .ctx = ctx,
        .is_compact_mode = is_compact_mode,
        .max_count = max_count,
        .status = REDISMODULE_OK,
        .actual_elements_count = 0
    };

    Info_ViewAllFinishedQueries(_reply_finished_queries, (void*)&user_data);

    if (actual_element_count) {
        *actual_element_count = user_data.actual_elements_count;
    }

    return user_data.status;
}

// Parses and handles the "GRAPH.INFO QUERIES PREV" command.
static int _parse_and_reply_info_queries_prev
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    const bool is_compact_mode,
    uint64_t *actual_element_count
) {
    ASSERT(ctx && argv && argc);
    if (!ctx || !argv || !argc) {
        RedisModule_ReplyWithError(ctx, INVALID_PARAMETERS_MESSAGE);
        return REDISMODULE_ERR;
    }

    // We expect the count as the last argument.
    const char *arg_count = RedisModule_StringPtrLen(argv[argc - 1], NULL);
    const long long max_count = MIN(atoll(arg_count), _info_queries_max_count());
    if (max_count > 0) {
        REDISMODULE_DO(_reply_with_queries_info_prev(
            ctx,
            is_compact_mode,
            max_count,
            actual_element_count
        ));
    }

    return REDISMODULE_OK;
}

// Parses and handles the "GRAPH.INFO QUERIES" command.
static int _reply_with_queries
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    const bool is_compact_mode,
    uint8_t *top_level_count
) {
    ASSERT(ctx);

    if (!ctx) {
        RedisModule_ReplyWithError(ctx, INVALID_PARAMETERS_MESSAGE);
        return REDISMODULE_ERR;
    }

    const InfoQueriesFlag flags = _parse_info_queries_flags_from_args(argv, argc);

    if (flags == InfoQueriesFlag_NONE) {
        return REDISMODULE_OK;
    }

    if (!is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, QUERIES_KEY_NAME));
    }

    if (*top_level_count) {
        // We count the array below.
        ++*top_level_count;
    }

    REDISMODULE_DO(RedisModule_ReplyWithArray(
        ctx,
        REDISMODULE_POSTPONED_LEN));

    uint64_t actual_element_count = 0;

    if (CHECK_FLAG(flags, InfoQueriesFlag_PREV)) {
        REDISMODULE_DO(_parse_and_reply_info_queries_prev(
            ctx,
            argv,
            argc,
            is_compact_mode,
            &actual_element_count
        ));
    }

    if (CHECK_FLAG(flags, InfoQueriesFlag_CURRENT)) {
        uint64_t element_count = 0;
        REDISMODULE_DO(_reply_with_queries_info_from_all_graphs(
            ctx,
            is_compact_mode,
            &element_count
        ));
        actual_element_count += element_count;
    }

    RedisModule_ReplySetArrayLength(ctx, actual_element_count);

    return REDISMODULE_OK;
}

// Handles the "GRAPH.INFO QUERIES" subcommand.
// The format is "GRAPH.INFO QUERIES [CURRENT] [PREV <count>]".
static int _info_queries
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    const bool is_compact_mode
) {
    ASSERT(ctx);
    if (!ctx) {
        RedisModule_ReplyWithError(ctx, INVALID_PARAMETERS_MESSAGE);
        return REDISMODULE_ERR;
    }

    uint8_t top_level_count = 1;

    REDISMODULE_DO(module_reply_map(
        ctx,
        is_compact_mode,
        REDISMODULE_POSTPONED_LEN));

    REDISMODULE_DO(_reply_with_queries_info_global(ctx, is_compact_mode));

    REDISMODULE_DO(_reply_with_queries(
        ctx,
        argv,
        argc,
        is_compact_mode,
        &top_level_count
    ));

    module_reply_map_set_postponed_length(
        ctx,
        is_compact_mode,
        top_level_count
    );

    return REDISMODULE_OK;
}

// Handles the "GRAPH.INFO GET" subcommand.
// The format is "GRAPH.INFO GET <key> [MEM] [COUNTS] [STAT]".
static int _info_get
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    const bool is_compact_mode
) {
    ASSERT(ctx);
    int result = REDISMODULE_ERR;

    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }

    const char *graph_name = RedisModule_StringPtrLen(argv[1], NULL);

    if (!graph_name) {
        RedisModule_ReplyWithError(ctx, ERROR_NO_GRAPH_NAME_SPECIFIED);
        return REDISMODULE_ERR;
    }
    const InfoGetFlag flags = _parse_info_get_flags_from_args(argv + 1, argc - 1);

    if (_string_equals_case_insensitive(graph_name, ALL_GRAPH_KEYS_MASK)) {
        return _get_all_graphs_info(ctx, is_compact_mode, flags);
    }

    return _get_graph_info(ctx, graph_name, is_compact_mode, flags);
}

// Handles the "GRAPH.INFO RESET" subcommand.
// The format is "GRAPH.INFO RESET <key>".
static int _info_reset
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx && argv);

    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }

    const char *graph_name = RedisModule_StringPtrLen(argv[1], NULL);

    if (!graph_name) {
        RedisModule_ReplyWithError(ctx, ERROR_NO_GRAPH_NAME_SPECIFIED);
        return REDISMODULE_ERR;
    }

    if (_string_equals_case_insensitive(graph_name, ALL_GRAPH_KEYS_MASK)) {
        return _reset_all_graphs_info(ctx);
    }

    return _reset_graph_info(ctx, graph_name);
}

// Attempts to find the specified subcommand of "GRAPH.INFO" and dispatch it.
// Returns true if the command was found and handled, false otherwise.
static bool _dispatch_subcommand
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    const char *subcommand_name,
    int *result,
    const bool is_compact_mode
) {
    ASSERT(ctx && result);
    ASSERT(subcommand_name != NULL && "Subcommand must be specified.");

    if (_is_queries_cmd(subcommand_name)) {
        *result = _info_queries(ctx, argv + 1, argc - 1, is_compact_mode);
    } else if (_is_get_cmd(subcommand_name)) {
        *result = _info_get(ctx, argv, argc, is_compact_mode);
    } else if (_is_reset_cmd(subcommand_name)) {
        *result = _info_reset(ctx, argv, argc);
    } else {
        return false;
    }

    return true;
}

// Dispatch the subcommand.
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

    if (!_is_cmd_info_enabled()) {
        RedisModule_ReplyWithError(ctx, "TODO");
        return REDISMODULE_ERR;
    }

    int result = REDISMODULE_ERR;

    const char *arg = RedisModule_StringPtrLen(argv[argc - 1], NULL);
    const bool is_compact_mode = _is_compact_mode(arg);

    const char *subcommand_name = RedisModule_StringPtrLen(argv[1], NULL);
    if (!_dispatch_subcommand(
        ctx,
        argv + 1,
        is_compact_mode ? argc - 2 : argc - 1,
        subcommand_name,
        &result,
        is_compact_mode)) {
        RedisModule_ReplyWithError(ctx, UNKNOWN_SUBCOMMAND_MESSAGE);
    }

    return result;
}
