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

// A wrapper for RedisModule_ functions which returns immediately on failure.
#define REDISMODULE_DO(doable) \
    do { \
        const int ret = doable; \
        ASSERT(ret == REDISMODULE_OK && "Redis module function " #doable " returned an error."); \
        if (ret != REDISMODULE_OK) { \
            return ret; \
        } \
    } while(0);

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

// Global info - across all the graphs available.
typedef struct {
    uint64_t max_query_pipeline_time;
    uint64_t total_waiting_queries_count;
    uint64_t total_executing_queries_count;
    uint64_t total_reporting_queries_count;
} GlobalInfo;

// Returns true if the strings are equal (case insensitively).
// NOTE: The strings must have a NULL-character at the end (strlen requirement).
static bool _string_equals_case_insensitive(const char *lhs, const char *rhs) {
    if (strlen(lhs) != strlen(rhs)) {
        return false;
    }
    return !strcasecmp(lhs, rhs);
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

static uint64_t _waiting_queries_count_from_graph
(
    const GraphContext *gc
) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetWaitingQueriesCount(&gc->info);
}

static uint64_t _executing_queries_count_from_graph
(
    const GraphContext *gc
) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetExecutingQueriesCount(&gc->info);
}

static uint64_t _reporting_queries_count_from_graph
(
    const GraphContext *gc
) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetReportingQueriesCount(&gc->info);
}

static uint64_t _max_query_pipeline_time_from_graph
(
    const GraphContext *gc
) {
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
    const uint64_t waiting_queries_count = _waiting_queries_count_from_graph(gc);
    const uint64_t executing_queries_count = _executing_queries_count_from_graph(gc);
    const uint64_t reporting_queries_count = _reporting_queries_count_from_graph(gc);
    const uint64_t max_query_pipeline_time = _max_query_pipeline_time_from_graph(gc);

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
    ASSERT(ctx);
    if (!ctx) {
        return REDISMODULE_ERR;
    }

    const long key_value_count = 4;
    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, key_value_count));
    // 1
    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, MAX_QUERY_PIPELINE_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, global_info.max_query_pipeline_time));
    // 2
    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, TOTAL_WAITING_QUERIES_COUNT_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, global_info.total_waiting_queries_count));
    // 3
    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, global_info.total_executing_queries_count));
    // 4
    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME));
    REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, global_info.total_reporting_queries_count));

    return REDISMODULE_OK;
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

// Breaks the encapsulation!
// TODO rewrite it so that it doesn't break the encapsulation!
// Currently it knows about the internals of the Info data structure
// and the way it stores the data.
static int _reply_graph_query_info_storage
(
    RedisModuleCtx *ctx,
    const QueryInfoStorage *storage
) {
    ASSERT(ctx);
    ASSERT(storage);
    if (!ctx || !storage) {
        return REDISMODULE_ERR;
    }

    const long key_value_count = 5;
    const uint32_t length = array_len(storage->queries);
    REDISMODULE_DO(RedisModule_ReplyWithArray(ctx, length));
    for (uint32_t i = 0; i < length; ++i) {
        const QueryInfo info = storage->queries[i];
        ASSERT(info.context);
        if (!info.context) {
            break;
        }
        REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, key_value_count));
        // 1
        // Note: customer proprietary data. should not appear in support packages
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Query"));
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, info.context->query_data.query));
        // 2
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Current total time (milliseconds)"));
        REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, QueryInfo_GetTotalTimeSpent(info, NULL)));
        // 3
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Current wait time (milliseconds)"));
        REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, QueryInfo_GetWaitingTime(info)));
        // 4
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Current execution time (milliseconds)"));
        REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, QueryInfo_GetExecutionTime(info)));
        // 5
        REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Current reporting time (milliseconds)"));
        REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, QueryInfo_GetReportingTime(info)));
        // TODO memory
        // // 6
        // REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Current processing memory (bytes)"));
        // REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, QueryInfo_GetReportingTime(info)));
        // // 7
        // REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Current undo-log memory (bytes)"));
        // REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, QueryInfo_GetReportingTime(info)));
        // // 8
        // REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Current result-set memory (bytes)"));
        // REDISMODULE_DO(RedisModule_ReplyWithLongLong(ctx, QueryInfo_GetReportingTime(info)));
    }

    return REDISMODULE_OK;
}

static int _reply_graph_info(RedisModuleCtx *ctx, const GraphContext *gc) {
    ASSERT(ctx);
    ASSERT(gc);
    if (!ctx || !gc) {
        return REDISMODULE_ERR;
    }

    const Info info = gc->info;

    REDISMODULE_DO(RedisModule_ReplyWithMap(ctx, 2));
    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Executing queries"));

    if (_reply_graph_query_info_storage(ctx, &info.executing_queries)) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Reporting queries"));

    if (_reply_graph_query_info_storage(ctx, &info.reporting_queries)) {
        return REDISMODULE_ERR;
    }

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

    if (_reply_global_info(ctx, global_info)) {
        return REDISMODULE_ERR;
    }

    REDISMODULE_DO(RedisModule_ReplyWithCString(ctx, "Per-graph data"));
    if (_reply_per_graph_data(ctx)) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

static int _reset_all_graphs_info(RedisModuleCtx *ctx) {
    ASSERT(ctx);
    ASSERT(graphs_in_keyspace);
    if (!ctx || !graphs_in_keyspace) {
        return REDISMODULE_ERR;
    }

    const uint32_t graphs_count = array_len(graphs_in_keyspace);

    for (uint32_t i = 0; i < graphs_count; ++i) {
        const GraphContext *gc = graphs_in_keyspace[i];
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
        const GraphContext *gc = graphs_in_keyspace[i];
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
    RedisModule_ReplyWithError(ctx, "Couldn't find the specified graph");
    return REDISMODULE_ERR;
}

// GRAPH.INFO QUERIES
static int _info_queries
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx != NULL);
    UNUSED(argv);
    UNUSED(argc);

    return _reply_with_queries_info_from_all_graphs(ctx);
}

// GRAPH.INFO GET
static int _info_get
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx != NULL);
    int result = REDISMODULE_OK;

    RedisModule_ReplyWithError(ctx, UNIMPLEMENTED_ERROR_STRING);

    return result;
}

// GRAPH.INFO RESET
static int _info_reset
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx != NULL);

    if (argc < 3) {
        return RedisModule_WrongArity(ctx);
    }

    const char *graph_name = RedisModule_StringPtrLen(argv[2], NULL);

    if (!graph_name) {
        RedisModule_ReplyWithError(ctx, "No graph name was specified");
        return REDISMODULE_ERR;
    }

    if (_string_equals_case_insensitive(graph_name, "*")) {
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

    if (_is_queries_cmd(subcommand_name)) {
        *result = _info_queries(ctx, argv, argc);
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
    if (!_dispatch_subcommand(ctx, argv, argc, subcommand_name, &result)) {
        RedisModule_ReplyWithError(ctx, UNKNOWN_SUBCOMMAND_MESSAGE);
    }

    return result;
}


