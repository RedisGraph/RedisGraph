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
    uint64_t waiting_time_milliseconds;
    // The time spent on executing.
    uint64_t executing_time_milliseconds;
    // The time spent on reporting.
    uint64_t reporting_time_milliseconds;
    // The context of the query.
    const struct QueryCtx *context;
    // The command context of the query.
    // When a command is received by the redis server it is dispatched to the
    // graph command dispatcher. At this moment, there is no query yet but only
    // the command context. Once the command dispatcher figures out the query
    // const struct CommandCtx *command;
} QueryInfo;

// Creates a new, empty query info object.
QueryInfo QueryInfo_New();
// Assigns the query context to the query info.
void QueryInfo_SetQueryContext(QueryInfo *, const struct QueryCtx *);
// Sets the query info object state.
void QueryInfo_SetState(QueryInfo *, const QueryState);
void QueryInfo_SetWaitingState(QueryInfo *);
void QueryInfo_SetExecutingState(QueryInfo *);
void QueryInfo_SetReportingState(QueryInfo *);
void QueryInfo_SetNextState(QueryInfo *);

// Informs the query info object that the execution has started.
void QueryInfo_SetAlreadyWaiting
(
    QueryInfo *,
    const uint64_t waiting_time_milliseconds
);

// Informs the query info object that the execution has started.
void QueryInfo_SetExecutionStarted
(
    QueryInfo *,
    const uint64_t waiting_time_milliseconds
);

// Informs the query info object that the reporting has started.
void QueryInfo_SetReportingStarted
(
    QueryInfo *,
    const uint64_t executing_time_milliseconds
);

// Informs the query info object that the reporting has finished.
void QueryInfo_SetReportingFinished
(
    QueryInfo *,
    const uint64_t executing_time_milliseconds
);

typedef struct {
    // Storage for query information.
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
