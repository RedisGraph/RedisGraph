/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include <stdatomic.h>
#include <stdbool.h>

typedef struct QueryCtx QueryCtx;

typedef struct {
    // The time it spent waiting.
    uint64_t waiting_time_milliseconds;
    // The time spent on executing.
    uint64_t executing_time_milliseconds;
    // The time spent on reporting.
    uint64_t reporting_time_milliseconds;
    // The context of the query.
    const QueryCtx *context;
    // The command context of the query.
    // When a command is received by the redis server it is dispatched to the
    // graph command dispatcher. At this moment, there is no query yet but only
    // the command context. Once the command dispatcher figures out the query
    // const struct CommandCtx *command;
} QueryInfo;

// Creates a new, empty query info object.
QueryInfo QueryInfo_New();
// Assigns the query context to the query info.
void QueryInfo_SetQueryContext(QueryInfo *, const QueryCtx *);
// Returns the total time spent by a query waiting, executing and reporting.
uint64_t QueryInfo_GetTotalTimeSpent(const QueryInfo, bool *is_ok);
// Returns the time the query spent waiting.
uint64_t QueryInfo_GetWaitingTime(const QueryInfo);
// Returns the time the query spent executing.
uint64_t QueryInfo_GetExecutionTime(const QueryInfo);
// Returns the time the query spent reporting.
uint64_t QueryInfo_GetReportingTime(const QueryInfo);

typedef struct {
    // Storage for query information for waiting queries.
    // This implementation uses the "arr.h" facility with a mutex.
    // TODO use lock-free skip list for storing queries.
    QueryInfo *queries;
} QueryInfoStorage;

// Creates a new query info storage.
QueryInfoStorage QueryInfoStorage_New();
// Clears the storage by removing all the queries stored.
void QueryInfoStorage_Clear(QueryInfoStorage *);
// Deallocates the storage.
void QueryInfoStorage_Free(QueryInfoStorage *);
// Returns the current length of the storage (in elements).
uint32_t QueryInfoStorage_Length(QueryInfoStorage *);
// Adds a query info object.
void QueryInfoStorage_Add(QueryInfoStorage *, const QueryInfo);
// Returns true if the element has successfully been removed.
bool QueryInfoStorage_Remove(QueryInfoStorage *, const QueryInfo *);
// Returns true if the element has successfully been removed.
bool QueryInfoStorage_RemoveByContext(QueryInfoStorage *, const QueryCtx *);

typedef struct {
    // Storage for query information for waiting queries.
    QueryInfoStorage waiting_queries;
    // Storage for query information for queries being executed.
    QueryInfoStorage executing_queries;
    // Storage for query information for reporting currently queries.
    QueryInfoStorage reporting_queries;
    // Maximum registered time a query was spent waiting, executing and
    // reporting the results.
    atomic_uint_fast64_t max_query_pipeline_time;
} Info;

// Create a new info structure.
Info Info_New();
// Reset an already existing info structure.
void Info_Reset(Info *);
// Free the info structure's contents.
void Info_Free(const Info);
// Insert a query information into the info structure. The query is supposed
// to be added right after being successfully parsed and known to the module,
// and before it starts being executed.
void Info_AddWaitingQueryInfo
(
    Info *,
    const QueryCtx *,
    const uint64_t waiting_time_milliseconds
);
// Indicates that the provided query has finished waiting and stated being
// executed.
void Info_IndicateQueryStartedExecution
(
    Info *,
    const QueryCtx *,
    const uint64_t waiting_time_milliseconds
);
// Indicates that the query has finished the execution and has started
// reporting the results back to the client.
void Info_IndicateQueryStartedReporting
(
    Info *,
    const QueryCtx *,
    const uint64_t executing_time_milliseconds
);
// Indicates that the query has finished reporting the results and is no longer
// required to be stored and kept track of.
void Info_IndicateQueryFinishedReporting
(
    Info *,
    const QueryCtx *,
    const uint64_t reporting_time_milliseconds
);
// Find a QueryInfo object by the provided context.
QueryInfo* Info_FindQueryInfo(Info *, const QueryCtx *);
// Return the total number of queries currently queued or being executed.
uint64_t Info_GetTotalQueriesCount(const Info *);
// Return the number of queries currently waiting to be executed.
uint64_t Info_GetWaitingQueriesCount(const Info *);
// Return the number of queries being currently executed.
uint64_t Info_GetExecutingQueriesCount(const Info *);
// Return the number of queries currently reporting results back.
uint64_t Info_GetReportingQueriesCount(const Info *);
// Return the maximum registered time a query was spent waiting, executing and
// reporting the results.
uint64_t Info_GetMaxQueryPipelineTime(const Info *);
