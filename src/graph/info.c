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

void _Info_ClearQueries(Info info) {
    if (array_len(info.query_info_array)) {
        array_free(info.query_info_array);
        array_clear(info.query_info_array);
    }
}

Info Info_New() {
    const Info info = {
        .query_info_array = array_new(QueryInfo, 10),
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
