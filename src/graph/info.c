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
#include "util/num.h"

#define INITIAL_QUERY_INFO_CAPACITY 100

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

#define REQUIRE_TRUE(arg_name) \
    ASSERT(arg_name && "#arg_name must be true."); \
    if (!arg_name) { \
        return; \
    }

#define REQUIRE_TRUE_OR_RETURN(arg_name, return_value) \
    ASSERT(arg_name && "#arg_name must be true."); \
    if (!arg_name) { \
        return return_value; \
    }

QueryInfo QueryInfo_New() {
    const QueryInfo query_info = {
        .waiting_time_milliseconds = 0,
        .executing_time_milliseconds = 0,
        .reporting_time_milliseconds = 0,
        .context = NULL
    };
    return query_info;
}

void QueryInfo_SetQueryContext
(
    QueryInfo *query_info,
    const struct QueryCtx *query_ctx
) {
    REQUIRE_ARG(query_info);

    query_info->context = query_ctx;
}

uint64_t QueryInfo_GetTotalTimeSpent(const QueryInfo info, bool *is_ok) {
    uint64_t total_time_spent = 0;

    if (!checked_add_u64(
        total_time_spent,
        info.waiting_time_milliseconds,
        &total_time_spent)) {
        // We have a value overflow.
        if (is_ok) {
            *is_ok = false;
            return 0;
        }
    }

    if (!checked_add_u64(
        total_time_spent,
        info.executing_time_milliseconds,
        &total_time_spent)) {
        // We have a value overflow.
        if (is_ok) {
            *is_ok = false;
            return 0;
        }
    }

    if (!checked_add_u64(
        total_time_spent,
        info.reporting_time_milliseconds,
        &total_time_spent)) {
        // We have a value overflow.
        if (is_ok) {
            *is_ok = false;
            return 0;
        }
    }

    return total_time_spent;
}

uint64_t QueryInfo_GetWaitingTime(const QueryInfo info) {
    return info.waiting_time_milliseconds;
}

uint64_t QueryInfo_GetExecutionTime(const QueryInfo info) {
    return info.executing_time_milliseconds;
}

uint64_t QueryInfo_GetReportingTime(const QueryInfo info) {
    return info.reporting_time_milliseconds;
}

QueryInfoStorage QueryInfoStorage_New() {
    QueryInfoStorage storage;
    storage.queries = array_new(QueryInfo, INITIAL_QUERY_INFO_CAPACITY);
    return storage;
}

void QueryInfoStorage_Clear(QueryInfoStorage *storage) {
    REQUIRE_ARG(storage);

    array_clear(storage->queries);
}

void QueryInfoStorage_Free(QueryInfoStorage *storage) {
    REQUIRE_ARG(storage);

    array_free(storage->queries);
}

uint32_t QueryInfoStorage_Length(QueryInfoStorage *storage) {
    REQUIRE_ARG_OR_RETURN(storage, 0);
    return array_len(storage->queries);
}

void QueryInfoStorage_Add(QueryInfoStorage *storage, const QueryInfo info) {
    REQUIRE_ARG(storage);
    // TODO check for duplicates?
    array_append(storage->queries, info);
}

bool QueryInfoStorage_Remove(QueryInfoStorage *storage, const QueryInfo *info) {
    return QueryInfoStorage_RemoveByContext(storage, info ? info->context : NULL);
}

bool QueryInfoStorage_RemoveByContext
(
    QueryInfoStorage *storage,
    const struct QueryCtx *context
) {
    REQUIRE_ARG_OR_RETURN(storage, false);
    REQUIRE_ARG_OR_RETURN(context, false);

    for (uint32_t i = 0; i < array_len(storage->queries); ++i) {
        QueryInfo query_info = storage->queries[i];
        if (query_info.context == context) {
            array_del(storage->queries, i);
            return true;
        }
    }

    return false;
}

static void _Info_ClearQueries(Info info) {
    QueryInfoStorage_Clear(&info.waiting_queries);
    QueryInfoStorage_Clear(&info.executing_queries);
    QueryInfoStorage_Clear(&info.reporting_queries);
}

Info Info_New() {
    const Info info = {
        .waiting_queries = QueryInfoStorage_New(),
        .executing_queries = QueryInfoStorage_New(),
        .reporting_queries = QueryInfoStorage_New(),
        .max_query_pipeline_time = 0
    };

    return info;
}

void Info_Reset(Info *info) {
    REQUIRE_ARG(info);

    _Info_ClearQueries(*info);
    info->max_query_pipeline_time = 0;
}

void Info_Free(Info info) {
    QueryInfoStorage_Free(&info.waiting_queries);
    QueryInfoStorage_Free(&info.executing_queries);
    QueryInfoStorage_Free(&info.reporting_queries);
}

void Info_AddWaitingQueryInfo
(
    Info *info,
    const struct QueryCtx *query_context,
    const uint64_t waiting_time_milliseconds
) {
    REQUIRE_ARG(info);

	QueryInfo query_info = QueryInfo_New();
	QueryInfo_SetQueryContext(&query_info, query_context);
    query_info.waiting_time_milliseconds += waiting_time_milliseconds;
    QueryInfoStorage_Add(&info->waiting_queries, query_info);
}

void Info_IndicateQueryStartedExecution
(
    Info *info,
    const struct QueryCtx *context, 
    const uint64_t waiting_time_milliseconds
) {
    REQUIRE_ARG(info);
    REQUIRE_ARG(context);

    // This effectively moves the query info object from the waiting queue
    // to the executing queue, recording the time spent waiting.

    // TODO abstract the work here and abstract away the iteration.
    // --- Synchronised start
    QueryInfoStorage storage = info->waiting_queries;
    for (uint32_t i = 0; i < array_len(storage.queries); ++i) {
        QueryInfo query_info = storage.queries[i];
        if (query_info.context == context) {
            query_info.waiting_time_milliseconds += waiting_time_milliseconds;
            array_del(storage.queries, i);
            QueryInfoStorage_Add(&info->executing_queries, query_info);

            break;
        }
    }
    // --- Synchronised end
}

void Info_IndicateQueryStartedReporting
(
    Info *info,
    const struct QueryCtx *context, 
    const uint64_t executing_time_milliseconds
) {
    REQUIRE_ARG(info);
    REQUIRE_ARG(context);

    // This effectively moves the query info object from the executing queue
    // to the reporting queue, recording the time spent executing.

    // TODO abstract the work here and abstract away the iteration.
    // --- Synchronised start
    QueryInfoStorage storage = info->executing_queries;
    for (uint32_t i = 0; i < array_len(storage.queries); ++i) {
        QueryInfo query_info = storage.queries[i];
        if (query_info.context == context) {
            query_info.executing_time_milliseconds += executing_time_milliseconds;
            array_del(storage.queries, i);
            QueryInfoStorage_Add(&info->reporting_queries, query_info);

            break;
        }
    }
    // --- Synchronised end
}

static void _Info_RecalculateMaxQueryWaitingTime
(
    Info *info,
    const QueryInfo query_info
) {
    REQUIRE_ARG(info);

    bool is_ok = true;
    const uint64_t total_query_time = QueryInfo_GetTotalTimeSpent(query_info, &is_ok);
    REQUIRE_TRUE(is_ok);
    info->max_query_pipeline_time = MAX(info->max_query_pipeline_time, total_query_time);
}

void Info_IndicateQueryFinishedReporting
(
    Info *info,
    const struct QueryCtx *context, 
    const uint64_t reporting_time_milliseconds
) {
    REQUIRE_ARG(info);
    REQUIRE_ARG(context);

    // This effectively removes the query info object from the reporting queue.

    // TODO abstract the work here and abstract away the iteration.
    // --- Synchronised start
    QueryInfoStorage storage = info->reporting_queries;
    for (uint32_t i = 0; i < array_len(storage.queries); ++i) {
        QueryInfo query_info = storage.queries[i];
        if (query_info.context == context) {
            query_info.reporting_time_milliseconds += reporting_time_milliseconds;
            array_del(storage.queries, i);
            _Info_RecalculateMaxQueryWaitingTime(info, query_info);

            break;
        }
    }
    // --- Synchronised end
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

uint64_t Info_GetTotalQueriesCount(const Info* info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    // --- Synchronised start
    const uint64_t waiting = Info_GetWaitingQueriesCount(info);
    const uint64_t executing = Info_GetExecutingQueriesCount(info);
    const uint64_t reporting = Info_GetReportingQueriesCount(info);
    // --- Synchronised end

    uint64_t total = waiting;
    REQUIRE_TRUE_OR_RETURN(checked_add_u64(total, executing, &total), 0);
    REQUIRE_TRUE_OR_RETURN(checked_add_u64(total, reporting, &total), 0);

    return total;
}

uint64_t Info_GetWaitingQueriesCount(const Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    return QueryInfoStorage_Length(&info->waiting_queries);
}

uint64_t Info_GetExecutingQueriesCount(const Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    return QueryInfoStorage_Length(&info->executing_queries);
}

uint64_t Info_GetReportingQueriesCount(const Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    return QueryInfoStorage_Length(&info->reporting_queries);
}

uint64_t Info_GetMaxQueryPipelineTime(const Info *info) {
    REQUIRE_ARG_OR_RETURN(info, 0);

    return info->max_query_pipeline_time;
}
