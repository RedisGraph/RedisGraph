/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "../util/arr.h"
#include "../redismodule.h"
#include "../graph/graphcontext.h"
#include "util/num.h"

#define SUBCOMMAND_NAME_QUERIES "QUERIES"
#define SUBCOMMAND_NAME_GET "GET"
#define SUBCOMMAND_NAME_RESET "RESET"
#define UNKNOWN_SUBCOMMAND_MESSAGE "Unknown subcommand."

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;

// Global info - across all the graphs available.
typedef struct {
    uint64_t max_query_waiting_time;
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

    return Info_GetWaitingQueriesCount(gc->info);
}

static uint64_t _executing_queries_count_from_graph
(
    const GraphContext *gc
) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetExecutingQueriesCount(gc->info);
}

static uint64_t _reporting_queries_count_from_graph
(
    const GraphContext *gc
) {
    ASSERT(gc != NULL);

    if (!gc) {
        return 0;
    }

    return Info_GetReportingQueriesCount(gc->info);
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

    if (!checked_add_u64(
        global_info->total_waiting_queries_count,
        waiting_queries_count,
        &global_info->total_waiting_queries_count)) {
        // We have a value overflow.
        if (is_ok) {
            return false;
        }
    }

    return true;
}

static int _reply_with_queries_info_from_all_graphs
(
    RedisModuleCtx *ctx
) {
	ASSERT(graphs_in_keyspace != NULL);
    if (!ctx || !graphs_in_keyspace) {
        return REDISMODULE_ERR;
    }

    bool is_ok = true;
	const uint graphs_count = array_len(graphs_in_keyspace);
    GlobalInfo global_info = {};

	for(uint i = 0; i < graphs_count; ++i) {
		const GraphContext *gc = graphs_in_keyspace[i];
        if (!gc) {
            RedisModule_ReplyWithError(ctx, "Graph does not exist.");
            return REDISMODULE_ERR;
        }

        is_ok = _collect_queries_info_from_graph(ctx, gc, &global_info);

        if (!is_ok) {
            RedisModule_ReplyWithError(ctx, "Error while collecting data.");
            return REDISMODULE_ERR;
        }
    }

    RedisModule_ReplyWithLongLong(ctx, global_info.max_query_waiting_time);
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

    RedisModule_ReplyWithNull(ctx);

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
    int result = REDISMODULE_OK;

    RedisModule_ReplyWithNull(ctx);

    return result;
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


