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

#define SUBCOMMAND_NAME_QUERIES "QUERIES"
#define SUBCOMMAND_NAME_GET "GET"
#define SUBCOMMAND_NAME_RESET "RESET"
#define COMPACT_MODE_OPTION "--compact"
#define UNKNOWN_SUBCOMMAND_MESSAGE "Unknown subcommand."
#define MAX_QUERY_WAIT_TIME_KEY_NAME "Max query wait duration (milliseconds)"
#define TOTAL_WAITING_QUERIES_COUNT_KEY_NAME "Total waiting queries count"
#define TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME "Total executing queries count"
#define TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME "Total reporting queries count"
#define GLOBAL_INFO_KEY_NAME "Global info"
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

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

// Global info - across all the graphs available.
typedef struct GlobalInfo {
    uint64_t max_query_wait_time_time;
    uint64_t total_waiting_queries_count;
    uint64_t total_executing_queries_count;
    uint64_t total_reporting_queries_count;
} GlobalInfo;

typedef enum QueryStage {
    QueryStage_WAITING = 0,
    QueryStage_EXECUTING,
    QueryStage_REPORTING
} QueryStage;

typedef enum InfoQueriesFlag {
    InfoQueriesFlag_NONE = 0,
    InfoQueriesFlag_CURRENT = 1,
    InfoQueriesFlag_PREV = 2,
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

static bool _is_cmd_info_enabled() {
	bool cmd_info_enabled = false;
	return Config_Option_get(Config_CMD_INFO, &cmd_info_enabled) && cmd_info_enabled;
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
    const char **argv,
    const int argc
) {
    InfoGetFlag flags = InfoGetFlag_NONE;

    if (!argv || argc <= 0) {
        return flags;
    }

    int read = 0;
    while (read < argc) {
        flags |= _parse_info_get_flag_from_string(argv[read]);
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

static bool _args_have_compact_mode_option(const RedisModuleString **argv, const int argc) {
    for (int i = 0; i < argc; ++i) {
        const char *arg = RedisModule_StringPtrLen(argv[i], NULL);
        if (_is_compact_mode(arg)) {
            return true;
        }
    }

    return false;
}

static uint64_t _waiting_queries_count_from_graph(const GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetWaitingQueriesCount(&gc->info);
}

static uint64_t _executing_queries_count_from_graph(const GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetExecutingQueriesCount(&gc->info);
}

static uint64_t _reporting_queries_count_from_graph(const GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetReportingQueriesCount(&gc->info);
}

static uint64_t _max_query_wait_time_from_graph(const GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetMaxQueryWaitTime(&gc->info);
}

static bool _collect_queries_info_from_graph
(
    RedisModuleCtx *ctx,
    const GraphContext *gc,
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

    if (!checked_add_u64(
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

static int _reply_global_info_compact
(
    RedisModuleCtx *ctx,
    const GlobalInfo global_info
) {
    static const long ITEM_COUNT = 4;

    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_DO(RedisModule_ReplyWithArray(ctx, ITEM_COUNT));
    // 1
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        global_info.max_query_wait_time_time));
    // 2
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        global_info.total_waiting_queries_count));
    // 3
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        global_info.total_executing_queries_count));
    // 4
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        global_info.total_reporting_queries_count));

    return REDISMODULE_OK;
}

static int _reply_global_info_full
(
    RedisModuleCtx *ctx,
    const GlobalInfo global_info
) {
    static const long KEY_VALUE_COUNT = 4;

    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, KEY_VALUE_COUNT));
    // 1
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        MAX_QUERY_WAIT_TIME_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        global_info.max_query_wait_time_time));
    // 2
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        TOTAL_WAITING_QUERIES_COUNT_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        global_info.total_waiting_queries_count));
    // 3
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        global_info.total_executing_queries_count));
    // 4
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
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
        const GraphContext *gc = graphs_in_keyspace[i];
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
        const GraphContext *gc = graphs_in_keyspace[i];
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

static int _reply_graph_query_info_compact
(
    RedisModuleCtx *ctx,
    const QueryStage query_stage,
    const QueryInfo info
) {
    static const long ITEM_COUNT = 8;

    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    const QueryCtx *query_ctx = QueryInfo_GetQueryContext(&info);
    ASSERT(query_ctx);
    if (!query_ctx) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_DO(RedisModule_ReplyWithArray(
        ctx,
        ITEM_COUNT));
    // 1
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        query_ctx->gc->graph_name));
    // 2
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        (long)query_stage));
    // 3
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetReceivedTimestamp(info)));
    // 4
    // Note: customer proprietary data. should not appear in support packages
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        query_ctx->query_data.query));
    // 5
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetTotalTimeSpent(info, NULL)));
    // 6
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetWaitingTime(info)));
    // 7
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetExecutionTime(info)));
    // 8
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetReportingTime(info)));
    // TODO memory
    // // 7
    // REDISMODULE_DO(RedisModule_ReplyWithCString(
    //     ctx,
    //     "Current processing memory (bytes)"));
    // REDISMODULE_DO(RedisModule_ReplyWithLongLong(
    //     ctx,
    //     QueryInfo_GetReportingTime(info)));
    // // 8
    // REDISMODULE_DO(RedisModule_ReplyWithCString(
    //     ctx,
    //     "Current undo-log memory (bytes)"));
    // REDISMODULE_DO(RedisModule_ReplyWithLongLong(
    //     ctx,
    //     QueryInfo_GetReportingTime(info)));
    // // 9
    // REDISMODULE_DO(RedisModule_ReplyWithCString(
    //     ctx,
    //     "Current result-set memory (bytes)"));
    // REDISMODULE_DO(RedisModule_ReplyWithLongLong(
    //     ctx,
    //     QueryInfo_GetReportingTime(info)));
}

static int _reply_graph_query_info_full
(
    RedisModuleCtx *ctx,
    const QueryStage query_stage,
    const QueryInfo info
) {
    static const long KEY_VALUE_COUNT = 8;

    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }
    const QueryCtx *query_ctx = QueryInfo_GetQueryContext(&info);
    ASSERT(query_ctx);
    if (!query_ctx) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, KEY_VALUE_COUNT));
    // 1
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Graph name"));
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        query_ctx->gc->graph_name));
    // 2
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Stage"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        (long)query_stage));
    // 3
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Receive timestamp (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetReceivedTimestamp(info)));
    // 4
    // Note: customer proprietary data. should not appear in support packages
    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Query"));
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        query_ctx->query_data.query));
    // 5
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Current total duration (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetTotalTimeSpent(info, NULL)));
    // 6
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Current wait duration (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetWaitingTime(info)));
    // 7
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Current execution duration (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetExecutionTime(info)));
    // 8
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Current reporting duration (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetReportingTime(info)));
    // TODO memory
    // // 7
    // REDISMODULE_DO(RedisModule_ReplyWithCString(
    //     ctx,
    //     "Current processing memory (bytes)"));
    // REDISMODULE_DO(RedisModule_ReplyWithLongLong(
    //     ctx,
    //     QueryInfo_GetReportingTime(info)));
    // // 8
    // REDISMODULE_DO(RedisModule_ReplyWithCString(
    //     ctx,
    //     "Current undo-log memory (bytes)"));
    // REDISMODULE_DO(RedisModule_ReplyWithLongLong(
    //     ctx,
    //     QueryInfo_GetReportingTime(info)));
    // // 9
    // REDISMODULE_DO(RedisModule_ReplyWithCString(
    //     ctx,
    //     "Current result-set memory (bytes)"));
    // REDISMODULE_DO(RedisModule_ReplyWithLongLong(
    //     ctx,
    //     QueryInfo_GetReportingTime(info)));

    return REDISMODULE_OK;
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
        if (is_compact_mode) {
            REDISMODULE_DO(_reply_graph_query_info_compact(ctx, query_stage, *info));
        } else {
            REDISMODULE_DO(_reply_graph_query_info_full(ctx, query_stage, *info));
        }
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
    const GraphContext *gc,
    const QueryStage query_stage,
    const uint64_t max_count,
    uint64_t *printed_count
) {
    ASSERT(ctx);
    ASSERT(gc);
    if (!ctx || !gc) {
        return REDISMODULE_ERR;
    }

    const Info *info = &gc->info;
    uint64_t iterated = 0;
    Info_Lock(info);

    QueryInfoStorage *storage = Info_GetWaitingQueriesStorage(info);
    switch (query_stage) {
        case QueryStage_EXECUTING: storage = Info_GetExecutingQueriesStorage(info); break;
        case QueryStage_REPORTING: storage = Info_GetReportingQueriesStorage(info); break;
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
        const GraphContext *gc = graphs_in_keyspace[i];
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
    const uint64_t max_elements_count
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

    REDISMODULE_DO(RedisModule_ReplyWithArray(
        ctx,
        REDISMODULE_POSTPONED_ARRAY_LEN));

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

    RedisModule_ReplySetArrayLength(ctx, actual_elements_count);

    return REDISMODULE_OK;
}

static int _reply_with_queries_info_from_all_graphs
(
    RedisModuleCtx *ctx,
    const bool is_compact_mode
) {
    static const long ITEM_COUNT = 2;

    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    GlobalInfo global_info = {};
    if (!_collect_global_info(ctx, &global_info)) {
        return REDISMODULE_ERR;
    }

    uint64_t max_elements_count = 0;
	if (!Config_Option_get(Config_CMD_INFO_MAX_QUERY_COUNT, &max_elements_count)) {
        max_elements_count = MAX_QUERIES_COUNT;
    }

    if (!is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, ITEM_COUNT));
    } else {
        REDISMODULE_DO(RedisModule_ReplyWithArray(ctx, ITEM_COUNT));
    }
    if (is_compact_mode) {
        REDISMODULE_DO(_reply_global_info_compact(ctx, global_info));
    } else {
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, GLOBAL_INFO_KEY_NAME));
        REDISMODULE_DO(_reply_global_info_full(ctx, global_info));
    }

    if (!is_compact_mode) {
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Queries"));
    }
    REDISMODULE_DO(_reply_graph_queries(ctx, is_compact_mode, max_elements_count));

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
        "Total number of edge property names"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.node_property_name_count));
    // 9
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Total number of node property names"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        info.edge_property_name_count));

    return REDISMODULE_OK;
}

// TODO add a compact mode version.
static int _reply_with_get_graph_info
(
    RedisModuleCtx *ctx,
    const GraphContext *gc,
    const InfoGetFlag flags
) {
    static const long KEY_VALUE_COUNT = 9;

    ASSERT(ctx);
    ASSERT(gc);
    ASSERT(gc->g);

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, KEY_VALUE_COUNT));
    // 1
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of nodes"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        Graph_NodeCount(gc->g) - Graph_DeletedNodeCount(gc->g)));
    // 2
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of relationships"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        Graph_EdgeCount(gc->g) - Graph_DeletedEdgeCount(gc->g)));
    // 3
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of node labels"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        Graph_LabelTypeCount(gc->g)));
    // 4
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of relationship types"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        Graph_RelationTypeCount(gc->g)));
    // 5
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of node indices"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        GraphContext_NodeIndexCount(gc)));
    // 6
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Number of relationship indices"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        GraphContext_EdgeIndexCount(gc)));
    // 7
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Total number of unique property names"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        GraphContext_AttributeCount(gc)));
    // 8
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Total number of edge property names"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        GraphContext_AllEdgePropertyNamesCount(gc)));
    // 9
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Total number of node property names"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        GraphContext_AllNodePropertyNamesCount(gc)));

    return REDISMODULE_OK;
}

static int _get_graph_info
(
    RedisModuleCtx *ctx,
    const char *graph_name,
    const InfoGetFlag flags
) {
    ASSERT(ctx);
    ASSERT(graph_name);

    const GraphContext *gc = _find_graph_with_name(graph_name);
    if (!gc) {
        RedisModule_ReplyWithError(ctx, ERROR_COULDNOT_FIND_GRAPH);
        return REDISMODULE_ERR;
    }

    return _reply_with_get_graph_info(ctx, gc, flags);
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

// TODO
static int _get_all_graphs_info_from_shards(RedisModuleCtx *ctx, const InfoGetFlag flags) {
    RedisModule_ReplyWithError(ctx, UNIMPLEMENTED_ERROR_STRING);
    return REDISMODULE_ERR;
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
    ASSERT(argv);

    return _reply_with_queries_info_from_all_graphs(ctx, is_compact_mode);
}

// GRAPH.INFO GET key [MEM] [COUNTS] [STAT]
static int _info_get
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
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
        return _get_all_graphs_info(ctx, flags);
    }

    return _get_graph_info(ctx, graph_name, flags);
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
        *result = _info_queries(ctx, argv, argc, is_compact_mode);
    } else if (_is_get_cmd(subcommand_name)) {
        *result = _info_get(ctx, argv, argc);
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


