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

#define SUBCOMMAND_NAME_QUERIES "QUERIES"
#define SUBCOMMAND_NAME_GET "GET"
#define SUBCOMMAND_NAME_RESET "RESET"
#define UNKNOWN_SUBCOMMAND_MESSAGE "Unknown subcommand."
#define MAX_QUERY_PIPELINE_KEY_NAME "Max Query Pipeline Time (milliseconds)"
#define TOTAL_WAITING_QUERIES_COUNT_KEY_NAME "Total waiting queries count"
#define TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME "Total executing queries count"
#define TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME "Total reporting queries count"
#define GLOBAL_INFO_KEY_NAME "Global info"
#define UNIMPLEMENTED_ERROR_STRING "Unimplemented"
#define INFO_GET_MEMORY_ARG "MEM"
#define INFO_GET_COUNTS_ARG "COUNTS"
#define INFO_GET_STATISTICS_ARG "STAT"
#define ERROR_COULDNOT_FIND_GRAPH "Couldn't find the specified graph"
#define ERROR_NO_GRAPH_NAME_SPECIFIED "No graph name was specified"
#define ALL_GRAPH_KEYS_MASK "*"

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

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

// Global info - across all the graphs available.
typedef struct GlobalInfo {
    uint64_t max_query_pipeline_time;
    uint64_t total_waiting_queries_count;
    uint64_t total_executing_queries_count;
    uint64_t total_reporting_queries_count;
} GlobalInfo;

typedef enum QueryStage {
    QueryStage_WAITING = 0,
    QueryStage_EXECUTING,
    QueryStage_REPORTING
} QueryStage;

typedef enum InfoGetFlag {
    InfoGetFlag_NONE = 0,
    InfoGetFlag_MEMORY = 1 << 0,
    InfoGetFlag_COUNTS = 1 << 1,
    InfoGetFlag_STATISTICS = 1 << 2,
} InfoGetFlag;

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

static bool _is_queries_cmd(const char *cmd) {
    return _string_equals_case_insensitive(cmd, SUBCOMMAND_NAME_QUERIES);
}

static bool _is_get_cmd(const char *cmd) {
    return _string_equals_case_insensitive(cmd, SUBCOMMAND_NAME_GET);
}

static bool _is_reset_cmd(const char *cmd) {
    return _string_equals_case_insensitive(cmd, SUBCOMMAND_NAME_RESET);
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

static uint64_t _max_query_pipeline_time_from_graph(const GraphContext *gc) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetMaxQueryPipelineTime(&gc->info);
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
    // TODO GraphContext Info
    const uint64_t waiting_queries_count = _waiting_queries_count_from_graph(gc);
    const uint64_t executing_queries_count = _executing_queries_count_from_graph(gc);
    const uint64_t reporting_queries_count = _reporting_queries_count_from_graph(gc);
    const uint64_t max_query_pipeline_time = _max_query_pipeline_time_from_graph(gc);

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
        global_info->max_query_pipeline_time,
        max_query_pipeline_time,
        &global_info->max_query_pipeline_time)) {
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
        MAX_QUERY_PIPELINE_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        global_info.max_query_pipeline_time));
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

static int _reply_graph_query_info
(
    RedisModuleCtx *ctx,
    const QueryInfo info
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    static const long KEY_VALUE_COUNT = 5;

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, KEY_VALUE_COUNT));
    // 1
    // Note: customer proprietary data. should not appear in support packages
    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Query"));
    const QueryCtx *query_ctx = QueryInfo_GetQueryContext(&info);
    ASSERT(query_ctx);
    if (!query_ctx) {
        return REDISMODULE_ERR;
    }
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        query_ctx->query_data.query));
    // 2
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Current total time (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetTotalTimeSpent(info, NULL)));
    // 3
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Current wait time (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetWaitingTime(info)));
    // 4
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Current execution time (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetExecutionTime(info)));
    // 5
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Current reporting time (milliseconds)"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        QueryInfo_GetReportingTime(info)));
    // TODO memory
    // // 6
    // REDISMODULE_DO(RedisModule_ReplyWithCString(
    //     ctx,
    //     "Current processing memory (bytes)"));
    // REDISMODULE_DO(RedisModule_ReplyWithLongLong(
    //     ctx,
    //     QueryInfo_GetReportingTime(info)));
    // // 7
    // REDISMODULE_DO(RedisModule_ReplyWithCString(
    //     ctx,
    //     "Current undo-log memory (bytes)"));
    // REDISMODULE_DO(RedisModule_ReplyWithLongLong(
    //     ctx,
    //     QueryInfo_GetReportingTime(info)));
    // // 8
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
    const QueryInfoStorage *storage
) {
    ASSERT(ctx);
    ASSERT(storage);
    if (!ctx || !storage) {
        return REDISMODULE_ERR;
    }

    QueryInfoIterator iterator = QueryInfoIterator_New(storage);
    REDISMODULE_DO(RedisModule_ReplyWithArray(
        ctx,
        REDISMODULE_POSTPONED_ARRAY_LEN));
    uint64_t actual_elements_count = 0;
    QueryInfo *info = NULL;
    while ((info = QueryInfoIterator_NextValid(&iterator)) != NULL) {
        _update_query_stage_timer(query_stage, info);
        ++actual_elements_count;
        REDISMODULE_DO(_reply_graph_query_info(ctx, *info));
    }
    RedisModule_ReplySetArrayLength(ctx, actual_elements_count);

    return REDISMODULE_OK;
}

static int _reply_graph_info(RedisModuleCtx *ctx, const GraphContext *gc) {
    ASSERT(ctx);
    ASSERT(gc);
    if (!ctx || !gc) {
        return REDISMODULE_ERR;
    }

    const Info *info = &gc->info;

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, 2));

    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Executing queries"));
    REDISMODULE_DO(_reply_graph_query_info_storage(
        ctx,
        QueryStage_EXECUTING,
        &info->executing_queries_per_thread));

    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Reporting queries"));
    REDISMODULE_DO(_reply_graph_query_info_storage(
        ctx,
        QueryStage_REPORTING,
        &info->reporting_queries_per_thread));

    return REDISMODULE_OK;
}

static int _reply_per_graph_data(RedisModuleCtx *ctx) {
    ASSERT(ctx);
    ASSERT(graphs_in_keyspace);
    if (!ctx || !graphs_in_keyspace) {
        return REDISMODULE_ERR;
    }

    const uint32_t graphs_count = array_len(graphs_in_keyspace);

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, graphs_count));
    for (uint32_t i = 0; i < graphs_count; ++i) {
        const GraphContext *gc = graphs_in_keyspace[i];
        if (!gc) {
            return REDISMODULE_ERR;
        }

        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, gc->graph_name));

        if (_reply_graph_info(ctx, gc)) {
            return REDISMODULE_ERR;
        }
    }

    return REDISMODULE_OK;
}

static int _reply_with_queries_info_from_all_graphs
(
    RedisModuleCtx *ctx
) {
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    GlobalInfo global_info = {};
    if (!_collect_global_info(ctx, &global_info)) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, 2));
    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, GLOBAL_INFO_KEY_NAME));
    REDISMODULE_DO(_reply_global_info(ctx, global_info));

    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Per-graph data"));
    REDISMODULE_DO(_reply_per_graph_data(ctx));

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

static int _reply_with_get_graph_info
(
    RedisModuleCtx *ctx,
    const GraphContext *gc,
    const InfoGetFlag flags
) {
    static const long KEY_VALUE_COUNT = 10;

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
        "Total number of unique node property names"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        GraphContext_UniqueNodePropertyNamesCount(gc)));
    // 8
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Total number of unique edge property names"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        GraphContext_UniqueEdgePropertyNamesCount(gc)));
    // 9
    REDISMODULE_DO(RedisModule_ReplyWithCString(
        ctx,
        "Total number of edge property names"));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(
        ctx,
        GraphContext_AllEdgePropertyNamesCount(gc)));
    // 10
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

// TODO
static int _get_all_graphs_info(RedisModuleCtx *ctx, const InfoGetFlag flags) {
    RedisModule_ReplyWithError(ctx, UNIMPLEMENTED_ERROR_STRING);
    return REDISMODULE_ERR;
}

// TODO
static int _get_all_graphs_info_from_shards(RedisModuleCtx *ctx, const InfoGetFlag flags) {
    RedisModule_ReplyWithError(ctx, UNIMPLEMENTED_ERROR_STRING);
    return REDISMODULE_ERR;
}

// GRAPH.INFO QUERIES
static int _info_queries(RedisModuleCtx *ctx) {
    ASSERT(ctx != NULL);

    return _reply_with_queries_info_from_all_graphs(ctx);
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
    int *result
) {
    ASSERT(ctx != NULL);
    ASSERT(subcommand_name != NULL && "Subcommand must be specified.");
    ASSERT(result);

    if (_is_queries_cmd(subcommand_name)) {
        *result = _info_queries(ctx);
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

    int result = REDISMODULE_ERR;

    const char *subcommand_name = RedisModule_StringPtrLen(argv[1], NULL);
    if (!_dispatch_subcommand(ctx, argv + 1, argc - 1, subcommand_name, &result)) {
        RedisModule_ReplyWithError(ctx, UNKNOWN_SUBCOMMAND_MESSAGE);
    }

    return result;
}


