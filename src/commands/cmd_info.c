/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "util/arr.h"
#include "util/num.h"
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
#define ERROR_COULDNOT_FIND_GRAPH "Couldn't find the specified graph"
#define ERROR_NO_GRAPH_NAME_SPECIFIED "No graph name was specified"
#define ERROR_VALUES_OVERFLOW "Some values have overflown"
#define ALL_GRAPH_KEYS_MASK "*"
// A duplicate of what is set in config.c
#define MAX_QUERIES_COUNT 10000

// TODO move to a common place.
// A wrapper for RedisModule_ functions which returns immediately on failure.
#define REDISMODULE_DO(doable) \
    do { \
        const int ret = doable; \
        ASSERT(ret == REDISMODULE_OK \
         && "Redis module function " #doable " returned an error."); \
        if (ret != REDISMODULE_OK) { \
            return ret; \
        } \
    } while(0);

#define CHECKED_ADD_OR_RETURN(lhs, rhs, return_on_error) \
    if (!checked_add_u64(lhs, rhs, &lhs)) { \
        return return_on_error; \
    }

// Replies with either a map or array, depending on the compact mode flag.
int _reply_map
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const long key_value_count
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    if (is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithArray(ctx, key_value_count));
    } else {
        REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, key_value_count));
    }

    return REDISMODULE_OK;
}

// Sets the postponed value either for a map or an array, depending on the
// compact mode flag.
void _reply_map_set_postponed_length
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const long length
) {
    ASSERT(ctx);
    ASSERT(length != REDISMODULE_POSTPONED_LEN);
    if (!ctx || length == REDISMODULE_POSTPONED_LEN) {
        return;
    }

    if (is_compact_mode) {
        RedisModule_ReplySetArrayLength(ctx, length);
    } else {
        RedisModule_ReplySetMapLength(ctx, length);
    }

    return;
}

// Replies with the key(C string)-value(long long) pair when the compact mode is
// off, and with just the value when the compact mode is on.
int _reply_key_value_number
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const long long value
) {
    ASSERT(ctx);
    ASSERT(key);

    if (!ctx || !key) {
        return REDISMODULE_ERR;
    }

    if (!is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, value));

    return REDISMODULE_OK;
}

// Replies with the key(C string)-value(long long) pair when the compact mode is
// off, and with just the value when the compact mode is on.
int _reply_key_value_numbers
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const int64_t *values,
    const size_t length
) {
    ASSERT(ctx);
    ASSERT(key);

    if (!ctx || !key) {
        return REDISMODULE_ERR;
    }
    if (!values || !length) {
        // Nothing to do.
        return REDISMODULE_OK;
    }

    if (!is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_DO(RedisModule_ReplyWithArray(ctx, length));
    for (size_t i = 0; i < length; ++i) {
        REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, values[i]));
    }

    return REDISMODULE_OK;
}

// Replies with the key(C string)-value(C string) pair when the compact mode is
// off, and with just the value when the compact mode is on.
int _reply_key_value_string
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const char *key,
    const char *value
) {
    ASSERT(ctx);
    ASSERT(key);
    ASSERT(value);

    if (!ctx || !key || !value) {
        return REDISMODULE_ERR;
    }

    if (!is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, key));
    }

    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, value));

    return REDISMODULE_OK;
}

// typedef struct ReplyRecorder {
//     RedisModuleCtx *ctx;
//     bool is_compact_mode;
//     uint64_t element_count;
// } ReplyRecorder;


// static ReplyRecorder ReplyRecorder_New
// (
//     RedisModuleCtx *ctx,
//     const bool is_compact_mode,
//     bool *has_started
// ) {
//     const ReplyRecorder recorder = {
//         .context = ctx,
//         .is_compact_mode = is_compact_mode,
//         .element_count = 0
//     };

//     const int status = _reply_map(
//         ctx,
//         is_compact_mode,
//         REDISMODULE_POSTPONED_LEN
//     );

//     ASSERT(status == REDISMODULE_OK);
//     if (has_started) {
//         *has_started = status == REDISMODULE_OK;
//     }

//     return recorder;
// }

// static int ReplyRecorder_AddNumber
// (
//     ReplyRecorder *recorder,
//     const char *key,
//     const long long value
// ) {
//     ASSERT(recorder);
//     if (!recorder || !recorder->ctx) {
//         return REDISMODULE_ERR;
//     }
//     _reply_key_value_number(
//         recorder.ctx,
//         recorder.is_compact_mode,

//     )
// }

// static void ReplyRecorder_Finish(const ReplyRecorder recorder) {
//     _reply_map_set_postponed_length(
//         recorder.ctx,
//         recorder.is_compact_mode,
//         recorder.element_count
//     );
// }

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

// Global info - across all the graphs available.
typedef struct GlobalInfo {
    millis_t max_query_wait_time_time;
    uint64_t total_waiting_queries_count;
    uint64_t total_executing_queries_count;
    uint64_t total_reporting_queries_count;
} GlobalInfo;

typedef enum QueryStage {
    QueryStage_WAITING = 0,
    QueryStage_EXECUTING,
    QueryStage_REPORTING,
    QueryStage_FINISHED,
} QueryStage;

typedef enum InfoQueriesFlag {
    InfoQueriesFlag_NONE = 0,
    InfoQueriesFlag_CURRENT = 1 << 0,
    InfoQueriesFlag_PREV = 1 << 1,
} InfoQueriesFlag;

typedef enum InfoGetFlag {
    InfoGetFlag_NONE = 0,
    InfoGetFlag_MEMORY = 1 << 0,
    InfoGetFlag_COUNTS = 1 << 1,
    InfoGetFlag_STATISTICS = 1 << 2,
} InfoGetFlag;

typedef struct AggregatedGraphGetInfo {
    uint64_t graph_count;
    uint64_t node_count;
    uint64_t relationship_count;
    uint64_t node_label_count;
    uint64_t relationship_type_count;
    uint64_t node_index_count;
    uint64_t relationship_index_count;
    uint64_t node_property_name_count;
    uint64_t edge_property_name_count;
} AggregatedGraphGetInfo;

typedef struct ViewFinishedQueriesCallbackData {
    RedisModuleCtx *ctx;
    bool is_compact_mode;
    uint64_t max_count;
    int status;
    uint64_t actual_elements_count;
} ViewFinishedQueriesCallbackData;

// The same structure is used for replies for the finished and non-finished
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
        max_elements_count = MAX_QUERIES_COUNT;
    }

    return max_elements_count;
}

static bool AggregatedGraphGetInfo_AddFromGraphContext
(
    AggregatedGraphGetInfo *info,
    const GraphContext *gc
) {
    ASSERT(info);
    ASSERT(gc);
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

static InfoQueriesFlag _parse_info_queries_flag_from_string(const char *str) {
    if (_string_equals_case_insensitive(str, INFO_QUERIES_CURRENT_ARG)) {
        return InfoQueriesFlag_CURRENT;
    } else if (_string_equals_case_insensitive(str, INFO_QUERIES_PREV_ARG)) {
        return InfoQueriesFlag_PREV;
    }
    return InfoQueriesFlag_NONE;
}

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

static uint64_t _waiting_queries_count_from_graph(GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetWaitingQueriesCount(&gc->info);
}

static uint64_t _executing_queries_count_from_graph(GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetExecutingQueriesCount(&gc->info);
}

static uint64_t _reporting_queries_count_from_graph(GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetReportingQueriesCount(&gc->info);
}

static uint64_t _max_query_wait_time_from_graph(GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetMaxQueryWaitTime(&gc->info);
}

static bool _collect_queries_info_from_graph
(
    RedisModuleCtx *ctx,
    GraphContext *gc,
    GlobalInfo *global_info
) {
    ASSERT(ctx != NULL);
    ASSERT(gc != NULL);
    ASSERT(global_info != NULL);
    if (!ctx || !gc || !global_info) {
        return false;
    }

    bool is_ok = true;
    // TODO GraphContext Info (get and use).
    const uint64_t waiting_queries_count = _waiting_queries_count_from_graph(gc);
    const uint64_t executing_queries_count = _executing_queries_count_from_graph(gc);
    const uint64_t reporting_queries_count = _reporting_queries_count_from_graph(gc);
    const uint64_t max_query_wait_time_time = _max_query_wait_time_from_graph(gc);

    // TODO let it overflow.
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


static int _reply_global_info
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const GlobalInfo global_info
) {
    static const long KEY_VALUE_COUNT = 4;

    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_DO(_reply_map(ctx, is_compact_mode, KEY_VALUE_COUNT));
    // 1
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        MAX_QUERY_WAIT_TIME_KEY_NAME,
        global_info.max_query_wait_time_time));
    // 2
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        TOTAL_WAITING_QUERIES_COUNT_KEY_NAME,
        global_info.total_waiting_queries_count));
    // 3
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME,
        global_info.total_executing_queries_count));
    // 4
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME,
        global_info.total_reporting_queries_count));

    return REDISMODULE_OK;
}

static GraphContext* _find_graph_with_name(const char *graph_name) {
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

static bool _collect_global_info(RedisModuleCtx *ctx, GlobalInfo *global_info) {
    ASSERT(ctx != NULL);
    ASSERT(global_info != NULL);
    ASSERT(graphs_in_keyspace != NULL);
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

static int _reply_graph_query_info
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const CommonQueryInfo info
) {
    static const long KEY_VALUE_COUNT = 8;

    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    const millis_t total_spent_time
        = info.wait_duration
        + info.execution_duration
        + info.report_duration;

    REDISMODULE_DO(_reply_map(ctx, is_compact_mode, KEY_VALUE_COUNT));
    // 1
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        RECEIVED_TIMESTAMP_KEY_NAME,
        info.received_at_ms));
    // 2
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        STAGE_KEY_NAME,
        (long long)info.stage));
    // 3
    REDISMODULE_DO(_reply_key_value_string(
        ctx,
        is_compact_mode,
        GRAPH_NAME_KEY_NAME,
        info.graph_name));
    // 4
    REDISMODULE_DO(_reply_key_value_string(
        ctx,
        is_compact_mode,
        QUERY_KEY_NAME,
        info.query_string));
    // 5
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        TOTAL_DURATION_KEY_NAME,
        total_spent_time));
    // 6
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        WAIT_DURATION_KEY_NAME,
        info.wait_duration));
    // 7
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        EXECUTION_DURATION_KEY_NAME,
        info.execution_duration));
    // 8
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        REPORT_DURATION_KEY_NAME,
        info.report_duration));

    return REDISMODULE_OK;
}

static int _reply_graph_query_info_storage
(
    RedisModuleCtx *ctx,
    const QueryStage query_stage,
    const QueryInfoStorage *storage,
    const bool is_compact_mode,
    const uint64_t max_count,
    uint64_t *iterated
) {
    ASSERT(ctx);
    ASSERT(storage);
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

static int _reply_with_graph_queries_of_stage
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    GraphContext *gc,
    const QueryStage query_stage,
    const uint64_t max_count,
    uint64_t *printed_count
) {
    ASSERT(ctx);
    ASSERT(gc);
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

static int _reply_queries_from_all_graphs
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const QueryStage query_stage,
    const uint64_t max_count,
    uint64_t *printed_count
) {
    ASSERT(ctx);
    ASSERT(graphs_in_keyspace);
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

static int _reply_graph_queries
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const uint64_t max_elements_count,
    uint64_t *actual_elements_count_ptr
) {
    ASSERT(ctx);
    ASSERT(graphs_in_keyspace);
    ASSERT(max_elements_count);
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

// TODO should we lock the graphs_in_keyspace?
static int _reset_all_graphs_info(RedisModuleCtx *ctx) {
    ASSERT(ctx);
    ASSERT(graphs_in_keyspace);
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

static int _reset_graph_info(RedisModuleCtx *ctx, const char *graph_name) {
    ASSERT(ctx);
    ASSERT(graphs_in_keyspace);
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
    RedisModule_ReplyWithError(ctx, ERROR_COULDNOT_FIND_GRAPH);
    return REDISMODULE_ERR;
}

static int _reply_with_get_aggregated_graph_info
(
    RedisModuleCtx *ctx,
    const AggregatedGraphGetInfo info
) {
    static const long KEY_VALUE_COUNT = 9;

    ASSERT(ctx);

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, KEY_VALUE_COUNT));
    // 1
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of graphs"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.graph_count));
    // 2
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of nodes"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.node_count));
    // 3
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of relationships"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.relationship_count));
    // 4
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of node labels"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.node_label_count));
    // 5
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of relationship types"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.relationship_type_count));
    // 6
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of node indices"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.node_index_count));
    // 7
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of relationship indices"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.relationship_index_count));
    // 8
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Total number of edge properties"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.node_property_name_count));
    // 9
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Total number of node properties"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.edge_property_name_count));

    return REDISMODULE_OK;
}

static int _reply_with_get_graph_info
(
    RedisModuleCtx *ctx,
    GraphContext *gc,
    const bool is_compact_mode,
    const InfoGetFlag flags
) {
    static const long KEY_VALUE_COUNT_DEFAULT = 9;
    long key_value_count = KEY_VALUE_COUNT_DEFAULT;

    ASSERT(ctx);
    ASSERT(gc);
    ASSERT(gc->g);

    REDISMODULE_DO(_reply_map(
        ctx,
        is_compact_mode,
        REDISMODULE_POSTPONED_LEN
    ));
    // 1
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        "Number of nodes",
        Graph_NodeCount(gc->g) - Graph_DeletedNodeCount(gc->g)
    ));
    // 2
    REDISMODULE_DO( _reply_key_value_number(
        ctx,
        is_compact_mode,
        "Number of relationships",
        Graph_EdgeCount(gc->g) - Graph_DeletedEdgeCount(gc->g)
    ));
    // 3
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        "Number of node labels",
        Graph_LabelTypeCount(gc->g)
    ));
    // 4
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        "Number of relationship types",
        Graph_RelationTypeCount(gc->g)
    ));
    // 5
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        "Number of node indices",
        GraphContext_NodeIndexCount(gc)
    ));
    // 6
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        "Number of relationship indices",
        GraphContext_EdgeIndexCount(gc)
    ));
    // 7
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        "Number of unique property names",
        GraphContext_AttributeCount(gc)
    ));
    // 8
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        "Total number of edge properties",
        GraphContext_AllEdgePropertyNamesCount(gc)
    ));
    // 9
    REDISMODULE_DO(_reply_key_value_number(
        ctx,
        is_compact_mode,
        "Total number of node properties",
        GraphContext_AllNodePropertyNamesCount(gc)
    ));

    /*
 [COUNTS]
Query counts since server start
(based on GRAPH.QUERY, GRAPH.RO_QUERY, GRAPH.PROFILE, GRAPH.EXPLAIN)
Total number of queries
Sum of the four below:
Number of successful read-only queries (queries with no intended side effect)
Number of successful write queries (create/update/delete)
Number of failed (except on timeout) read-only queries (queries with no intended side effect)
Number of failed (except on timeout) write queries (create/update/delete)
Number of timeout read-only queries (queries with no intended side effect)
Number of timeout write queries (create/update/delete)
    */
    if (CHECK_FLAG(flags, InfoGetFlag_COUNTS)) {
        static const long KEY_VALUE_COUNT_COUNTS_FLAG = 7;

        key_value_count += KEY_VALUE_COUNT_COUNTS_FLAG;
        const FinishedQueryCounters counters
            = Info_GetFinishedQueryCounters(gc->info);

        // 1
        REDISMODULE_DO(_reply_key_value_number(
            ctx,
            is_compact_mode,
            "Total number of queries",
            FinishedQueryCounters_GetTotalCount(counters)
        ));
        // 2
        REDISMODULE_DO(_reply_key_value_number(
            ctx,
            is_compact_mode,
            "Successful read-only queries",
            counters.readonly_succeeded_count
        ));
        // 3
        REDISMODULE_DO(_reply_key_value_number(
            ctx,
            is_compact_mode,
            "Successful write queries",
            counters.write_succeeded_count
        ));
        // 4
        REDISMODULE_DO(_reply_key_value_number(
            ctx,
            is_compact_mode,
            "Failed read-only queries",
            counters.readonly_failed_count
        ));
        // 5
        REDISMODULE_DO(_reply_key_value_number(
            ctx,
            is_compact_mode,
            "Failed write queries",
            counters.write_failed_count
        ));
        // 6
        REDISMODULE_DO(_reply_key_value_number(
            ctx,
            is_compact_mode,
            "Timed out read-only queries",
            counters.readonly_timedout_count
        ));
        // 7
        REDISMODULE_DO(_reply_key_value_number(
            ctx,
            is_compact_mode,
            "Timed out write queries",
            counters.write_timedout_count
        ));
    }

    /*
 [STAT]
Statistics regarding queries since server start (for one graph / total for all graphs):
For each of the following: 0.25, 0.5, 0.75, 0.9, 0.95, 0.99 quantiles  (using t-digest)
Query total duration (sum of the three below)
Query wait duration
Query execution duration
Query report duration
Query total memory (sum of the three below)
Query processing memory
Query undo-log memory
Query result-size memory
    */
    if (CHECK_FLAG(flags, InfoGetFlag_STATISTICS)) {
        static const long KEY_VALUE_COUNT_COUNTS_FLAG = 4;

        key_value_count += KEY_VALUE_COUNT_COUNTS_FLAG;
        const Percentiles percentiles
            = Info_GetDurationsPercentiles(&gc->info);

        // 1
        REDISMODULE_DO(_reply_key_value_numbers(
            ctx,
            is_compact_mode,
            "Query total durations",
            percentiles.total_durations,
            sizeof(percentiles.total_durations) / sizeof(percentiles.total_durations[0])
        ));

        // 2
        REDISMODULE_DO(_reply_key_value_numbers(
            ctx,
            is_compact_mode,
            "Query wait durations",
            percentiles.wait_durations,
            sizeof(percentiles.wait_durations) / sizeof(percentiles.wait_durations[0])
        ));

        // 3
        REDISMODULE_DO(_reply_key_value_numbers(
            ctx,
            is_compact_mode,
            "Query execution durations",
            percentiles.execution_durations,
            sizeof(percentiles.execution_durations) / sizeof(percentiles.execution_durations[0])
        ));

        // 4
        REDISMODULE_DO(_reply_key_value_numbers(
            ctx,
            is_compact_mode,
            "Query report durations",
            percentiles.report_durations,
            sizeof(percentiles.report_durations) / sizeof(percentiles.report_durations[0])
        ));

        // TODO memory
    }

    _reply_map_set_postponed_length(ctx, is_compact_mode, key_value_count);

    return REDISMODULE_OK;
}

static int _get_graph_info
(
    RedisModuleCtx *ctx,
    const char *graph_name,
    const bool is_compact_mode,
    const InfoGetFlag flags
) {
    ASSERT(ctx);
    ASSERT(graph_name);

    GraphContext *gc = _find_graph_with_name(graph_name);
    if (!gc) {
        RedisModule_ReplyWithError(ctx, ERROR_COULDNOT_FIND_GRAPH);
        return REDISMODULE_ERR;
    }

    return _reply_with_get_graph_info(ctx, gc, is_compact_mode, flags);
}

// TODO use flags
// TODO add compact mode.
static int _get_all_graphs_info
(
    RedisModuleCtx *ctx,
    const InfoGetFlag flags
) {
    ASSERT(ctx);
    const uint32_t graphs_count = array_len(graphs_in_keyspace);
    AggregatedGraphGetInfo info = {};

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

    return _reply_with_get_aggregated_graph_info(ctx, info);
}

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

static int _reply_with_queries_info_prev
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode,
    const uint64_t max_count,
    uint64_t *actual_element_count
) {
    ASSERT(ctx);
    ASSERT(max_count);
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

// GRAPH.INFO QUERIES [CURRENT] [PREV <count>]
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

    REDISMODULE_DO(_reply_map(
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

    _reply_map_set_postponed_length(
        ctx,
        is_compact_mode,
        top_level_count
    );

    return REDISMODULE_OK;
}

// GRAPH.INFO GET key [MEM] [COUNTS] [STAT]
static int _info_get
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    const bool is_compact_mode
) {
    ASSERT(ctx != NULL);
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
        // TODO add compact flag.
        return _get_all_graphs_info(ctx, flags);
    }

    return _get_graph_info(ctx, graph_name, is_compact_mode, flags);
}

// GRAPH.INFO RESET [name]
static int _info_reset
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx != NULL);
    ASSERT(argv);

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

static bool _dispatch_subcommand
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc,
    const char *subcommand_name,
    int *result,
    const bool is_compact_mode
) {
    ASSERT(ctx != NULL);
    ASSERT(subcommand_name != NULL && "Subcommand must be specified.");
    ASSERT(result);

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
    ASSERT(ctx != NULL);

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
