/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdatomic.h>

typedef enum {
    // The time since the server has received the query and before the
    // execution has started.
    QueryState_WAITING = 0,
    // The time since the execution has started until it finishes and before the
    // reporting starts.
    QueryState_EXECUTING = 1,
    // The time it took for the query to report the results back to the client.
    QueryState_REPORTING = 2
} QueryState;

typedef struct {
    // Current state of the query.
    QueryState state;
    // The time it spent waiting.
    uint64_t waiting_time;
    // The time spent on executing.
    uint64_t executing_time;
    // The time spent on reporting.
    uint64_t reporting_time;
    // The context of the query.
    struct QueryCtx *context;
} QueryInfo;

typedef struct {
    // An array containing query information.
    QueryInfo *query_info_array;
    // Maximum registered time a query was waiting for.
    atomic_uint_fast64_t max_query_waiting_time;
} Info;

// Create a new info structure.
Info Info_New();
// Reset an already existing info structure.
void Info_Reset(Info *);
// Free the info structure's contents.
void Info_Free(const Info);
// Insert a query information into the info structure.
void Info_AddQueryInfo(Info *, const QueryInfo);
// Remove the query information by the query context.
void Info_RemoveQueryInfo(Info *, const struct QueryCtx *);
// Find a QueryInfo object by the provided context.
QueryInfo* Info_FindQueryInfo(Info *, const struct QueryCtx *);
// Return the total number of queries currently queued or being executed.
uint64_t Info_GetTotalQueriesCount(const Info);
// Return the number of queries currently waiting to be executed.
uint64_t Info_GetWaitingQueriesCount(const Info);
// Return the number of queries being currently executed.
uint64_t Info_GetExecutingQueriesCount(const Info);
// Return the number of queries currently reporting results back.
uint64_t Info_GetReportingQueriesCount(const Info);
