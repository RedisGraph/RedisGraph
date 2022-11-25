/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

/*
 * This file contains the useful statistics and generic information about a
 * a graph. This information is used by the "GRAPH.INFO" command.
*/
#include "info.h"

#include "util/arr.h"
#include "query_ctx.h"

#define REQUIRE_ARG_OR_RETURN(arg_name, return_value) \
    ASSERT(arg_name && "#arg_name must be provided."); \
    if (!arg_name) { \
        return return_value; \
    }

#define REQUIRE_ARG(arg_name) \
    ASSERT(arg_name && "#arg_name must be provided."); \
    if (!arg_name) { \
        return; \
    }

QueryInfo QueryInfo_New() {
    const QueryInfo query_info = {
        .state = QueryState_WAITING,
        .waiting_time_milliseconds = 0,
        .executing_time_milliseconds = 0,
        .reporting_time_milliseconds = 0,
        .context = NULL
    };
    return query_info;
}

void QueryInfo_SetQueryContext(QueryInfo *query_info, const struct QueryCtx *query_ctx) {
    REQUIRE_ARG(query_info);

    query_info->context = query_ctx;
}

void QueryInfo_SetState(QueryInfo *query_info, const QueryState state) {
    REQUIRE_ARG(query_info);
    query_info->state = state;
}

void QueryInfo_SetWaitingState(QueryInfo *query_info) {
    REQUIRE_ARG(query_info);
	query_info->state = QueryState_WAITING;
}

void QueryInfo_SetExecutingState(QueryInfo *query_info) {
    REQUIRE_ARG(query_info);
	query_info->state = QueryState_EXECUTING;
}

void QueryInfo_SetReportingState(QueryInfo *query_info) {
    REQUIRE_ARG(query_info);
	query_info->state = QueryState_REPORTING;
}

void QueryInfo_SetNextState(QueryInfo *query_info) {
    REQUIRE_ARG(query_info);
    ASSERT(query_info->state != QueryState_REPORTING);
    if (query_info->state == QueryState_REPORTING) {
        return;
    }
    ++query_info->state;
}

void QueryInfo_SetAlreadyWaiting
(
    QueryInfo *query_info,
    const uint64_t waiting_time_milliseconds
) {
    REQUIRE_ARG(query_info);
    QueryInfo_SetWaitingState(query_info);
	query_info->waiting_time_milliseconds = waiting_time_milliseconds;
}

void QueryInfo_SetExecutionStarted
(
    QueryInfo *query_info,
    const uint64_t waiting_time_milliseconds
) {
    REQUIRE_ARG(query_info);
    QueryInfo_SetExecutingState(query_info);
	query_info->waiting_time_milliseconds += waiting_time_milliseconds;
}

void QueryInfo_SetReportingStarted
(
    QueryInfo *query_info,
    const uint64_t executing_time_milliseconds
) {
    REQUIRE_ARG(query_info);
	query_info->state = QueryState_REPORTING;
	query_info->executing_time_milliseconds += executing_time_milliseconds;
}

void QueryInfo_SetReportingFinished
(
    QueryInfo *query_info,
    const uint64_t reporting_time_milliseconds
) {
    REQUIRE_ARG(query_info);
    ASSERT(query_info->state == QueryState_REPORTING);
	query_info->reporting_time_milliseconds += reporting_time_milliseconds;
}

void _Info_ClearQueries(Info info) {
    if (array_len(info.query_info_array)) {
        array_free(info.query_info_array);
        array_clear(info.query_info_array);
    }
}

Info Info_New() {
    const Info info = {
        .query_info_array = array_new(QueryInfo, 100),
        .max_query_waiting_time = 0
    };

    return info;
}

void Info_Reset(Info *info) {
    REQUIRE_ARG(info);

    _Info_ClearQueries(*info);
    info->max_query_waiting_time = 0;
}

void Info_Free(Info info) {
    _Info_ClearQueries(info);
}

void Info_AddQueryInfo(Info *info, const QueryInfo query_info) {
    REQUIRE_ARG(info);

    // TODO Check if we actually need to add it (avoid duplicates).
    array_append(info->query_info_array, query_info);
}

void Info_RemoveQueryInfo(Info *info, const struct QueryCtx *query_ctx) {
    REQUIRE_ARG(info);
    REQUIRE_ARG(query_ctx);

    // TODO the code must work fast, and a simple array won't provide us with
    // a fast access based on QueryCtx, it will be a O(N) lookup every time.
    // For every update of the query information based on the QueryCtx pointer,
    // we are going to spend O(N). Perhaps, it is better to use a radix tree?
}

QueryInfo* Info_FindQueryInfo
(
    Info *info,
    const struct QueryCtx *query_ctx
) {
    REQUIRE_ARG_OR_RETURN(info, NULL);
    REQUIRE_ARG_OR_RETURN(query_ctx, NULL);

    // TODO
    return NULL;
}

uint64_t Info_GetTotalQueriesCount(const Info info) {
    // TODO
    return 0;
}

uint64_t Info_GetWaitingQueriesCount(const Info info) {
    // TODO
    return 0;
}

uint64_t Info_GetExecutingQueriesCount(const Info info) {
    // TODO
    return 0;
}

uint64_t Info_GetReportingQueriesCount(const Info info) {
    // TODO
    return 0;
}
