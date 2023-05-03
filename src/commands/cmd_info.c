/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

 // this file contains the implementation of the GRAPH.INFO command

#include <ctype.h>
#include <stdlib.h>

#include "RG.h"
#include "util/arr.h"
#include "util/num.h"
#include "query_ctx.h"
#include "util/module.h"
#include "redismodule.h"
#include "../info/info.h"
#include "util/thpool/pools.h"
#include "graph/graphcontext.h"
#include "configuration/config.h"

#define QUERYINFO_NUMFIELDS 8
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
#define UTILIZED_CACHE "Utilized cache"
#define GRAPH_NAME_KEY_NAME "Graph name"
#define SUBCOMMAND_NAME_QUERIES "QUERIES"
#define INFO_QUERIES_CURRENT_ARG "CURRENT"
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

// returns true if graph info tracking is enabled
static bool _is_cmd_info_enabled(void) {
    bool enabled = false;
    return Config_Option_get(Config_CMD_INFO, &enabled) && enabled;
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

// parses and validates the arguments for the "GRAPH.INFO QUERIES" command
// valid arguments are specified in the Graph_Info function
static int _parse_and_validate(
    RedisModuleCtx *ctx,             // context
    const RedisModuleString **argv,  // arguments
    int argc,                        // number of arguments
    InfoQueriesFlag *flags,          // [OUTPUT] flags (CURRENT, PREV <cnt>)
    long long *prev_cnt              // [OUTPUT] the <cnt> parameter for the "PREV" flag
) {
    // parse and validate the arguments 
    while(argc-- > 0) {
        const char *arg = RedisModule_StringPtrLen(argv[argc], NULL);
        InfoQueriesFlag flag = _parse_info_queries_flag_from_string(arg);
        if(flag == InfoQueriesFlag_NONE) {
            RedisModule_ReplyWithError(ctx, "unsupported flag");
            return REDISMODULE_ERR;
        } else if(flag == InfoQueriesFlag_PREV) {
            // the next argument should be the <cnt> parameter
            if(argc == 0) {
                // missing <cnt> parameter, reply with an error
                RedisModule_ReplyWithError(ctx, "missing <cnt> parameter");
                return REDISMODULE_ERR;
            }

            // validate the <cnt> parameter > 0
            const char *cnt_str = RedisModule_StringPtrLen(argv[argc], NULL);
            if(RedisModule_StringToLongLong(argv[argc],
                prev_cnt) != REDISMODULE_OK) {
                    RedisModule_ReplyWithError(ctx, "invalid <cnt> parameter");
                    return REDISMODULE_ERR;
            }
            argc--;
        }
        *flags |= flag;
    }

    return REDISMODULE_OK;
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

    uint64_t executing = 0;
    uint64_t reporting = 0;
    uint64_t waiting = Info_GetWaitingQueriesCount(gc->info);
    Info_GetExecutingReportingQueriesCount(gc->info, &executing, &reporting);
    uint64_t max_wait_time = Info_GetMaxQueryWaitTime(gc->info);

	global_info->total_waiting_queries_count   += waiting;
	global_info->total_executing_queries_count += executing;
	global_info->total_reporting_queries_count += reporting;

	global_info->max_query_wait_time = MAX(max_wait_time,
			global_info->max_query_wait_time);
}

// collects all the global information from all the graphs
static void _collect_global_info
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

        _collect_queries_info_from_graph(ctx, gc, global_info);
    }
}

// replies with the global information about the graphs
// the output is a part of the "GRAPH.INFO QUERIES" command
static void _reply_global_info
(
    RedisModuleCtx *ctx,
    const GlobalInfo *global_info
) {
    ASSERT(ctx != NULL);
    ASSERT(global_info != NULL);

    int res = RedisModule_ReplyWithArray(ctx, 2 * 4);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, MAX_QUERY_WAIT_TIME_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithLongLong(ctx, global_info->max_query_wait_time);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx,
        TOTAL_WAITING_QUERIES_COUNT_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithLongLong(ctx,
        global_info->total_waiting_queries_count);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx,
        TOTAL_EXECUTING_QUERIES_COUNT_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithLongLong(ctx,
        global_info->total_executing_queries_count);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx,
        TOTAL_REPORTING_QUERIES_COUNT_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithLongLong(ctx,
        global_info->total_reporting_queries_count);
    ASSERT(res == REDISMODULE_OK);
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

    REDISMODULE_ASSERT(ReplyRecorder_AddBool(
        &recorder,
        UTILIZED_CACHE,
        info.utilized_cache
    ))

    return REDISMODULE_OK;
}

// replies with the relevant information stored in qi
static void _reply_finished_query
(
    RedisModuleCtx *ctx,
    QueryInfo *qi
) {
    RedisModule_ReplyWithArray(ctx, 2 * QUERYINFO_NUMFIELDS);

    // 4) 1)  1) "Received at"
    //    2) (integer) 1682939141691
    //    3) "Stage"
    //    4) (integer) 3
    //    5) "Graph name"
    //    6) "g"
    //    7) "Query"
    //    8) "create (:N)"
    //    9) "Total duration"
    //   10) (integer) 0
    //   11) "Wait duration"
    //   12) (integer) 0
    //   13) "Execution duration"
    //   14) (integer) 0
    //   15) "Report duration"
    //   16) (integer) 0


    int res = RedisModule_ReplyWithCString(ctx, RECEIVED_TIMESTAMP_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithLongLong(ctx, qi->received_ts);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, STAGE_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithLongLong(ctx, qi->stage);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, GRAPH_NAME_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithCString(ctx, qi->graph_name);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, QUERY_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithCString(ctx, qi->query_string);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, TOTAL_DURATION_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithLongLong(ctx, qi->wait_duration + qi->execution_duration + qi->report_duration);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, WAIT_DURATION_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithLongLong(ctx, qi->wait_duration);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, EXECUTION_DURATION_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithLongLong(ctx, qi->execution_duration);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, REPORT_DURATION_KEY_NAME);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithLongLong(ctx, qi->report_duration);
    ASSERT(res == REDISMODULE_OK);

    res = RedisModule_ReplyWithCString(ctx, UTILIZED_CACHE);
    ASSERT(res == REDISMODULE_OK);
    res = RedisModule_ReplyWithBool(ctx, qi->utilized_cache);
    ASSERT(res == REDISMODULE_OK);
}

// returns a QueryInfo with the information stored in entry
static QueryInfo *_collect_query_info_from_stream_entry
(
    RedisModuleCallReply *entry
) {
    // TBD

    // beginning:
    // size_t num_fields = RedisModule_CallReplyLength(entry) - 1;
    // for (size_t j = 0; j < num_fields; j++) {
    //     RedisModuleString *field = RedisModule_CreateStringFromCallReply(RedisModule_CallReplyArrayElement(entry, j+1));
    //     RedisModule_ReplyWithString(ctx, field);
    // }
    

    // Just for compilation!
    return QueryInfo_New();
}

static void _collect_current_queries
(
    const GraphContext *gc,
    QueryInfo **current_queries,
    int *current_queries_count
) {
    Info *info = gc->info;
    // lock info
    Info_Lock(info);

    // append clones of the working QueryInfo's
    int n_threads = ThreadPools_ThreadCount() + 1;
    for(int i = 0; i < n_threads; i++) {
        QueryInfo *qi = *(QueryInfo**)array_elem(info->working_queries, i);
        if(qi != NULL && *current_queries_count < n_threads) {
            current_queries[*current_queries_count++] = QueryInfo_Clone(qi);
        }
    }

    // unlock info
    Info_Unlock(info);
}

// handles the "GRAPH.INFO QUERIES" subcommand
// the format is "GRAPH.INFO QUERIES [CURRENT] [PREV <count>]"
    // TODO: Once things are working without the additional key (<key> / *), change
    // to "GRAPH.INFO QUERIES <key>/* [CURRENT] [PREV <count>]"
static int _info_queries
(
    RedisModuleCtx *ctx,
    const RedisModuleString **argv,
    int argc
) {
    // an example for a command and reply:
    // command:
    // GRAPH.INFO QUERIES CURRENT PREV 5
    // reply:
    // "Global info"
    //     "Current maximum query wait duration"
    //     "Total waiting queries count"
    //     "Total executing queries count"
    //     "Total reporting queries count"
    // "Queries"
    //     [
    //         "Received at"
    //          "Stage"
    //          "Graph name"
    //          "Query"
    //          "Total duration"
    //          "Wait duration"
    //          "Execution duration"
    //          "Report duration"
    //          "Cache utilized"
    //     ] X n

    ASSERT(ctx != NULL);

    InfoQueriesFlag flags = InfoQueriesFlag_NONE;
    long long prev_cnt = 0;

    // parse arguments and validate the command
    if (_parse_and_validate(ctx, argv, argc, &flags, &prev_cnt) ==
            REDISMODULE_ERR) {
                return REDISMODULE_ERR;
    }

    // set high-level response array
        // Global Info
        // Queries (if CURRENT or PREV <cnt> where specified)
    bool only_global = (flags == InfoQueriesFlag_NONE);
    only_global ? RedisModule_ReplyWithArray(ctx, 2 * 1) 
                : RedisModule_ReplyWithArray(ctx, 2 * 2);

    // -------------------------------------------------------------------------
    // global information
    // -------------------------------------------------------------------------

    GlobalInfo global_info;
    _collect_global_info(ctx, &global_info);

    // reply with "Global Info"
    RedisModule_ReplyWithCString(ctx, "Global info");

    // reply with global information
    _reply_global_info(ctx, &global_info);

    if(!only_global) {
        RedisModule_ReplyWithCString(ctx, "Queries");
    }

    // -------------------------------------------------------------------------
    // CURRENT queries information
    // -------------------------------------------------------------------------
    long long current_cnt = 0;
    int n_threads = ThreadPools_ThreadCount() + 1;
    QueryInfo *current_queries[n_threads];
    int current_queries_count = 0;
    if(CHECK_FLAG(flags, InfoQueriesFlag_CURRENT)) {

        const uint32_t graphs_count = array_len(graphs_in_keyspace);
        // traverse graphs, collecting working queries information
        for (uint32_t i = 0; i < graphs_count &&
            current_queries_count < n_threads; ++i) {
                GraphContext *gc = graphs_in_keyspace[i];
                ASSERT(gc != NULL);

                _collect_current_queries(gc, current_queries,
                    &current_queries_count);
        }

        // set current_cnt to the number of CURRENT queries to reply
        current_cnt = array_len(current_queries);
    }

    // -------------------------------------------------------------------------
    // PREV queries information
    // -------------------------------------------------------------------------
    if(CHECK_FLAG(flags, InfoQueriesFlag_PREV)) {
        // ---------------------------------------------------------------------
        // iterate over the last prev_cnt elements from the stream, collecting
        // the relevant information
        // ---------------------------------------------------------------------
        RedisModuleCallReply *reply = RedisModule_Call(ctx, "XREVRANGE",
            "ccccl", GRAPH_INFO_STREAM_NAME, "+", "-", "COUNT", prev_cnt);

        // Check if the reply is valid
        if (RedisModule_CallReplyType(reply) != REDISMODULE_REPLY_ARRAY) {
            RedisModule_ReplyWithError(ctx, "Error reading stream");
            return REDISMODULE_ERR;
        }

        // make sure prev_cnt elements were read
        prev_cnt = (long long)RedisModule_CallReplyLength(reply);

        // Iterate over the entries in the reply
        for (int i = 0; i < prev_cnt; i++) {
            RedisModuleCallReply *entry = RedisModule_CallReplyArrayElement(reply, i);

            // collect data from the entry
            // TODO: See if we can just use RedisModule_CallReplyWithReply and return that instead of parsing to a qi!!
            // Just need to see how the current blends with it, and deal with the response length.
            // One important thing: The entry-id will be returned as well - 
                // But this is fine since we want to introduce a query-id per-query (will be saved 
                // in the QueryInfo struct) so we will later set it. At the moment, work with the entry-id.
                // So TODO: Add an incremental query-id to the QueryInfo struct.
            QueryInfo *qi = _collect_query_info_from_stream_entry(entry);
            _reply_finished_query(ctx, qi);
        }

        // Free the reply object
        RedisModule_FreeCallReply(reply);
    }

    // set response array for queries information
    if(!only_global) {
        RedisModule_ReplyWithArray(ctx, current_cnt + prev_cnt);

        // reply with the CURRENT queries
        if(current_queries_count > 0) {
            for(int i = 0; i < current_cnt; i++) {
                _reply_finished_query(ctx, current_queries[i]);
            }
        }
    }

    return REDISMODULE_OK;
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
    ASSERT(ctx    != NULL);
    ASSERT(result != NULL);
    ASSERT(subcmd != NULL);

	// GRAPH.INFO QUERIES
    if (!strcasecmp(subcmd, SUBCOMMAND_NAME_QUERIES)) {
        *result = _info_queries(ctx, argv + 1, (int)argc - 1);
    } else {
		// sub command either un-familiar or not supported
        return false;
    }

    return true;
}

// TODO: Update to the new API (<key> or * specified) once things work well with this one.
// GRAPH.INFO QUERIES
// GRAPH.INFO QUERIES CURRENT
// GRAPH.INFO QUERIES PREV num
// GRAPH.INFO QUERIES CURRENT PREV num
// dispatch the subcommand
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

